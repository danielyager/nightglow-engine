//
// Created by danie on 11/3/2026.
//

#include "d3dUtil.h"
//#include "MeshGen.h"
//#include "LoadM3d.h"
#include <comdef.h>
#include <fstream>

using Microsoft::WRL::ComPtr;

ComPtr<ID3D12Resource> d3dUtil::CreateDefaultBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,
    const void* initData, UINT64 byteSize, ComPtr<ID3D12Resource>& uploadBuffer) {

    ComPtr<ID3D12Resource> defaultBuffer;
    return defaultBuffer;
}

std::wstring DxException::ToString() const {
    // Get the string description of the error code.
    _com_error err(ErrorCode);
    std::wstring msg = err.ErrorMessage();

    return FunctionName + L" failed in " + Filename + L"; line " + std::to_wstring(LineNumber) + L"; error: " + msg;
}
