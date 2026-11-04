//
// Created by danie on 11/3/2026.
//
#pragma once

#ifndef NIGHTGLOW_ENGINE_VERTEXTYPES_H
#define NIGHTGLOW_ENGINE_VERTEXTYPES_H

#endif //NIGHTGLOW_ENGINE_VERTEXTYPES_H

#include <d3d12.h>
#include <DirectXMath.h>
#include <array>

struct VertexPosColor {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT4 color;
};



namespace Layouts {
    inline constexpr std::array<D3D12_INPUT_ELEMENT_DESC, 2> PosColor =
    {{
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,
          0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
          12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    }};

    inline constexpr D3D12_INPUT_LAYOUT_DESC PosColorDesc =
    { PosColor.data(), static_cast<UINT>(PosColor.size()) };
}