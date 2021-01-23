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

struct TextureObject
{
	ID3D11Texture2D* tp{nullptr};
	ID3D11ShaderResourceView* srv{nullptr};
	ID3D11RenderTargetView* rtv{nullptr};
};

struct ShaderObject
{
	ID3D11InputLayout* il{nullptr};
	ID3D11VertexShader* vs{nullptr};
	ID3D11PixelShader* ps{nullptr};
};

enum RasterizationState : uint8
{
	RS_NORMAL = 0,
	RS_DEBUG,

	RS_COUNT
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
	void initRasterizationsStates();

	void resizeBackBuffer(float width, float height);

	void destroyZBuffer();

	void setRasterizationState(RasterizationState t_State = RS_DEBUG);
	void setShaders(ShaderType t_Shader);
	void setVertexBuffer(VBObject t_buffer, uint32 offset = 0);
	void setIndexBuffer(IBObject t_buffer);
	void setViewport(float x, float y, float width, float height);

	void updateCBs();

	TextureObject createTexute(uint16 t_Width, uint16 t_Height, void* t_Data, uint64 t_Length)
	{
		TextureObject to;

		D3D11_TEXTURE2D_DESC desc;
		desc.Width = t_Width;
		desc.Height = t_Height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = t_Data;

		Device->CreateTexture2D(&desc, &data, &to.tp);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		
		Device->CreateShaderResourceView(to.tp, &srvDesc, &to.srv);

		return to;
	}
	
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
	// not sure which data has to be inlined though; what is that we use most commonly
	// together?
	ID3D11Device* Device{ nullptr };
	IDXGISwapChain* Swap{ nullptr };
	ID3D11DeviceContext* Context{ nullptr };
	
	ID3D11RenderTargetView* RenderTargetView{ nullptr };
	ID3D11DepthStencilView* DepthStencilView{ nullptr };

	ID3D11RasterizerState* rasterizationsStates[RS_COUNT];

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
