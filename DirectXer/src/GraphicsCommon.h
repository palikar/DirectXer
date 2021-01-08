#pragma once

#include <DirectXMath.h>
#include <d3d11.h>

#include "Types.h"

namespace dx = DirectX;

struct PSConstantBuffer
{
    inline static ID3D11Buffer* id{nullptr};
    
    dx::XMVECTOR color = {1.0f, 1.0f, 1.0f, 1.0f};
};

struct VSConstantBuffer
{
	inline static ID3D11Buffer* id{nullptr};
    
	dx::XMMATRIX model{dx::XMMatrixIdentity()};
	dx::XMMATRIX view{dx::XMMatrixIdentity()};
	dx::XMMATRIX projection{dx::XMMatrixIdentity()};
};

struct SimpleVertex
{
	struct
	{
		float x;
		float y;
		float z;
	} pos;
};
