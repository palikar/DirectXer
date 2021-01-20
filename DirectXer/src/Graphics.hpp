#pragma once

#include "IncludeWin.hpp"
#include "Logging.hpp"

#include <d3d11.h>
#include <wrl.h>

#include <vector>
#include <array>
#include <cmath>
#include <sstream>

#include <d3dcompiler.h>
#include <DirectXMath.h>

// @Fix (Stanislav) : this is so dumb. Visual Studio is fightning me yet again
#include "../../DXError/src/dxerr.h"

#include "GraphicsCommon.hpp"
#include "Types.hpp"


#ifndef NDEBUG
#define GFX_CALL(hrcall) do						\
	{											\
		gDxgiManager.ResetMessage();			\
		if( FAILED( hr = (hrcall) ) )			\
		{										\
			DXLOGHRESULT(hr);					\
			gDxgiManager.PrintMessages();		\
			abort();							\
		}										\
	}while(false)
#else
#define GFX_CALL(hrcall) (hrcall)
#endif

namespace dx = DirectX;

struct VBObject
{
	uint32 structSize;
	ID3D11Buffer* id{nullptr};
};

struct IBObject
{
	ID3D11Buffer* id{nullptr};
};

struct ShaderObject
{
	ID3D11InputLayout* il{nullptr};
	ID3D11VertexShader* vs{nullptr};
	ID3D11PixelShader* ps{nullptr};
};

class Graphics
{
  public:

	enum ShaderType
	{
		SHADER_SIMPLE = 0,

		SHADER_COUNT
	};

	enum TopolgyType : uint8
	{
		TT_TRIANGLES = 0,
		TT_LINES     = 1
	};


	void initSwapChain(HWND hWnd, float t_Width, float t_Height);
	void initBackBuffer();
	void initZBuffer(float width, float height);
	void initResources();

	void resizeBackBuffer(float width, float height);

	void destroyZBuffer();

	void setRasterizationState();
	void setShaders(ShaderType t_Shader);
	void setVertexBuffer(VBObject t_buffer, uint32 offset = 0);
	void setIndexBuffer(IBObject t_buffer);
	void setViewport(float x, float y, float width, float height);

	void updateCBs();

	VBObject createVertexBuffer(uint32 structSize, void* data, uint32 dataSize);
	IBObject createIndexBuffer(void* data, uint32 dataSize);
	template<typename Type, bool isPSBuffer>
	void createConstantBuffer(Type& buffer);

	void drawIndex(TopolgyType topology, uint32 count, uint32 offset = 0,  uint32 base = 0);

	void EndFrame();
	void ClearBuffer(float red, float green, float blue);
	void ClearZBuffer();

	void Destroy();

  public:
	// @Note: We should probably have a maximum of 8 pointers here (1 cahce line)
	ID3D11Device* Device{ nullptr };
	IDXGISwapChain* Swap{ nullptr };
	ID3D11DeviceContext* Context{ nullptr };
	
	ID3D11RenderTargetView* RenderTargetView{ nullptr };
	ID3D11DepthStencilView* DepthStencilView{ nullptr };

	std::array<ShaderObject, SHADER_COUNT> m_Shaders;

	PSConstantBuffer m_PixelShaderCB;
	VSConstantBuffer m_VertexShaderCB;	

};

template<typename VertexType>
VBObject vertexBufferFactory(Graphics& graphics, std::vector<VertexType> t_VertexList)
{
	return graphics.createVertexBuffer(sizeof(VertexType), t_VertexList.data(), (uint32)(sizeof(VertexType) * t_VertexList.size()));
}

template<typename IndexType = uint32>
IBObject indexBufferFactory(Graphics& graphics, std::vector<IndexType> t_IndexList)
{
	return graphics.createIndexBuffer(t_IndexList.data(), (uint32)(sizeof(IndexType) * t_IndexList.size()));
}
