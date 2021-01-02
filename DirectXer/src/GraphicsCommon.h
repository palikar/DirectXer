#include <DirectXMath.h>
#include <d3d11.h>


namespace dx = DirectX;

struct PSConstantBuffer
{
    inline static ID3D11Buffer* id{nullptr};
    
    float color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
};

struct VSConstantBuffer
{
	inline static ID3D11Buffer* id{nullptr};
    
	dx::XMMATRIX transform{dx::XMMatrixIdentity()};
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
