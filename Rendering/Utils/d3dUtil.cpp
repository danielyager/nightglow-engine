//
// Created by danie on 11/3/2026.
//

#include "d3dUtil.h"
//#include "MeshGen.h"
//#include "LoadM3d.h"
#include <d3d12.h>
#include <wrl.h>
#include <cstring>    // memcpy
#include <stdexcept>  // std::runtime_error
#include <comdef.h>
#include <fstream>

// Note: this assumes d3dUtil is a class/namespace declared elsewhere with this
// function declared in it. No d3dx12.h helpers (CD3DX12_*) are used; every
// struct is filled in by hand so you can see exactly what is happening.

// ---------------------------------------------------------------------------
// OVERVIEW
//
// A "default" heap lives in GPU-local memory. It is the fastest memory for the
// GPU to read, but the CPU CANNOT write to it directly. To get data into it:
//
//   1. Create an "upload" heap buffer (CPU-writable, GPU-readable) as a
//      staging area.
//   2. Copy your data from system memory into the upload buffer (CPU side).
//   3. Record a GPU command that copies upload buffer -> default buffer.
//   4. Use resource barriers so the GPU knows when the default buffer is
//      being written to and when it is ready to be read.
//
// IMPORTANT: Step 3 is only *recorded* on the command list. The copy happens
// later, when you execute the command list. The upload buffer must therefore
// stay alive until the GPU finishes. That is why the upload buffer is returned
// to the caller through the uploadBuffer out-parameter: the caller keeps it
// alive, and releases it after the command list has finished executing
// (e.g. after waiting on a fence).
// ---------------------------------------------------------------------------
Microsoft::WRL::ComPtr<ID3D12Resource> d3dUtil::CreateDefaultBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,
    const void* initData, UINT64 byteSize, Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer) {

    // This is the resource we will return: the final GPU-only buffer.
    Microsoft::WRL::ComPtr<ID3D12Resource> defaultBuffer;

    // -----------------------------------------------------------------------
    // A buffer is described with a D3D12_RESOURCE_DESC. Both of our buffers
    // (default and upload) use the same description, so we fill it in once.
    // -----------------------------------------------------------------------
    D3D12_RESOURCE_DESC bufferDesc = {};
    bufferDesc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER; // a plain buffer, not a texture
    bufferDesc.Alignment          = 0;                               // 0 = use default alignment (64KB)
    bufferDesc.Width              = byteSize;                        // for buffers, Width is the size in bytes
    bufferDesc.Height             = 1;                               // buffers must have Height = 1
    bufferDesc.DepthOrArraySize   = 1;                               // buffers must have this = 1
    bufferDesc.MipLevels          = 1;                               // buffers must have this = 1
    bufferDesc.Format             = DXGI_FORMAT_UNKNOWN;             // buffers have no pixel format
    bufferDesc.SampleDesc.Count   = 1;                               // no multisampling
    bufferDesc.SampleDesc.Quality = 0;
    bufferDesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;  // required layout for buffers
    bufferDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;        // no special usage flags

    // -----------------------------------------------------------------------
    // STEP 1: Create the DEFAULT heap buffer (the real, final GPU buffer).
    //
    // Heap properties say WHERE the memory lives. DEFAULT = GPU-local memory.
    // -----------------------------------------------------------------------
    D3D12_HEAP_PROPERTIES defaultHeapProps = {};
    defaultHeapProps.Type                 = D3D12_HEAP_TYPE_DEFAULT;
    defaultHeapProps.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN; // must be UNKNOWN for non-custom heaps
    defaultHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;       // must be UNKNOWN for non-custom heaps
    defaultHeapProps.CreationNodeMask     = 1;                               // single-GPU: node 0 -> mask 1
    defaultHeapProps.VisibleNodeMask      = 1;

    // "Committed" resource = D3D allocates a heap AND the resource in one call.
    // This is the simplest way to create a resource.
    //
    // Initial state COMMON is the neutral starting state. We will transition it
    // to COPY_DEST below before writing to it.
    HRESULT hr = device->CreateCommittedResource(
        &defaultHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,                                  // optimized clear value: only for render targets/depth buffers
        IID_PPV_ARGS(defaultBuffer.GetAddressOf()));
    if (FAILED(hr))
        throw std::runtime_error("CreateDefaultBuffer: failed to create default heap buffer");

    // -----------------------------------------------------------------------
    // STEP 2: Create the UPLOAD heap buffer (the CPU-writable staging buffer).
    //
    // UPLOAD heap memory can be written by the CPU and read by the GPU. It is
    // slower for the GPU to read than DEFAULT memory, which is why we only use
    // it as a stepping stone.
    // -----------------------------------------------------------------------
    D3D12_HEAP_PROPERTIES uploadHeapProps = {};
    uploadHeapProps.Type                 = D3D12_HEAP_TYPE_UPLOAD;
    uploadHeapProps.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    uploadHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    uploadHeapProps.CreationNodeMask     = 1;
    uploadHeapProps.VisibleNodeMask      = 1;

    // Resources in an UPLOAD heap MUST be created in the GENERIC_READ state
    // and can never change from it. That is fine: the GPU only reads from it.
    hr = device->CreateCommittedResource(
        &uploadHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,                              // same size/description as the default buffer
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(uploadBuffer.GetAddressOf())); // handed back to the caller to keep alive
    if (FAILED(hr))
        throw std::runtime_error("CreateDefaultBuffer: failed to create upload heap buffer");

    // -----------------------------------------------------------------------
    // STEP 3: Copy the data from system memory into the upload buffer (CPU).
    //
    // To write to a GPU resource from the CPU, we "map" it, which gives us a
    // CPU pointer to its memory. Then memcpy, then unmap.
    // -----------------------------------------------------------------------
    void* mappedData = nullptr;

    // The read range {0, 0} tells D3D "the CPU will not read from this
    // resource", which avoids unnecessary work. We only write.
    D3D12_RANGE readRange = { 0, 0 };

    hr = uploadBuffer->Map(0, &readRange, &mappedData);
    if (FAILED(hr))
        throw std::runtime_error("CreateDefaultBuffer: failed to map upload buffer");

    // Copy the caller's data into the upload buffer.
    std::memcpy(mappedData, initData, static_cast<size_t>(byteSize));

    // Unmap now that we are done writing. (Upload heap memory is fine to leave
    // mapped for its whole life, but unmapping here keeps things simple.)
    // Passing nullptr for the written range means "the whole resource may have
    // been written".
    uploadBuffer->Unmap(0, nullptr);

    // -----------------------------------------------------------------------
    // STEP 4: Record the GPU commands that move the data into the default buffer.
    //
    // Nothing below executes immediately. These calls only add commands to
    // cmdList. The GPU performs them after the caller calls
    // ExecuteCommandLists on this command list.
    // -----------------------------------------------------------------------

    // 4a. Barrier: COMMON -> COPY_DEST.
    //
    // D3D12 makes YOU tell the GPU how each resource is about to be used.
    // Before we can copy INTO the default buffer, it must be in COPY_DEST state.
    D3D12_RESOURCE_BARRIER toCopyDest = {};
    toCopyDest.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION; // a state transition
    toCopyDest.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    toCopyDest.Transition.pResource   = defaultBuffer.Get();                    // which resource
    toCopyDest.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES; // a buffer has just one subresource
    toCopyDest.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
    toCopyDest.Transition.StateAfter  = D3D12_RESOURCE_STATE_COPY_DEST;
    cmdList->ResourceBarrier(1, &toCopyDest);

    // 4b. The copy itself: upload buffer -> default buffer.
    //
    // CopyBufferRegion(dest, destOffset, source, sourceOffset, numBytes)
    // Both offsets are 0 because we copy the whole buffer from the start.
    cmdList->CopyBufferRegion(
        defaultBuffer.Get(), 0,
        uploadBuffer.Get(),  0,
        byteSize);

    // 4c. Barrier: COPY_DEST -> GENERIC_READ.
    //
    // The copy is finished, so tell the GPU the buffer is now going to be read
    // (e.g. as a vertex buffer, index buffer or constant buffer data).
    // GENERIC_READ is a combination of all the common read-only states, so it
    // works for vertex/index buffers, constant buffers, and so on.
    D3D12_RESOURCE_BARRIER toGenericRead = {};
    toGenericRead.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    toGenericRead.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    toGenericRead.Transition.pResource   = defaultBuffer.Get();
    toGenericRead.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    toGenericRead.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    toGenericRead.Transition.StateAfter  = D3D12_RESOURCE_STATE_GENERIC_READ;
    cmdList->ResourceBarrier(1, &toGenericRead);

    // -----------------------------------------------------------------------
    // DONE. Return the default buffer.
    //
    // REMINDER: the copy has only been RECORDED. The caller must:
    //   1. Close and execute the command list,
    //   2. Wait for the GPU to finish (fence),
    //   3. THEN release uploadBuffer.
    // Releasing uploadBuffer earlier can crash or corrupt data, because the GPU
    // may not have performed the copy yet.
    // -----------------------------------------------------------------------
    return defaultBuffer;
}

std::wstring DxException::ToString() const {
    // Get the string description of the error code.
    _com_error err(ErrorCode);
    std::wstring msg = err.ErrorMessage();

    return FunctionName + L" failed in " + Filename + L"; line " + std::to_wstring(LineNumber) + L"; error: " + msg;
}
