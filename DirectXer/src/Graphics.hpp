#pragma once

#include "IncludeWin.hpp"
#include "Logging.hpp"
#include "Memory.hpp"
#include "Math.hpp"

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


enum BlendingState : uint8
{
	BS_AlphaBlending = 0,
	BS_PremultipliedAlpha = 1,

	BS_Count,

};


class Graphics
{
  public:

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
	void initSamplers();
	void initBlending();

	void resizeBackBuffer(float width, float height);

	void bindTexture(uint32 t_Slot, TextureObject t_Texture);

	void bindPSConstantBuffers(CBObject* t_Buffers, uint16 t_Count, uint16 t_StartSlot);
	void bindVSConstantBuffers(CBObject* t_Buffers, uint16 t_Count, uint16 t_StartSlot);

	void setRasterizationState(RasterizationState t_State = RS_DEBUG);
	void setVertexBuffer(VBObject t_buffer, uint32 offset = 0);
	void setIndexBuffer(IBObject t_buffer);
	void setViewport(float x, float y, float width, float height);
	void setShaderConfiguration(ShaderConfig t_Confing);
	void setBlendingState(BlendingState t_State);

	TextureObject createTexture(uint16 t_Width, uint16 t_Height, TextureFormat t_Format, const void* t_Data, uint64 t_Length);
	TextureObject createCubeTexture(uint16 t_Width, uint16 t_Height, TextureFormat t_Format, void* t_Data[6]);
	VBObject createVertexBuffer(uint32 structSize, void* data, uint32 dataSize, bool dynamic = false);
	IBObject createIndexBuffer(void* data, uint32 dataSize, bool dynamic = false);
	CBObject createConstantBuffer(uint32 t_Size, void* t_InitData);

	void updateCBs();
	void updateCBs(CBObject& t_CbObject, uint32 t_Length, void* t_Data);
	void updateVertexBuffer(VBObject t_Buffer, void* data, uint64 t_Length);
	void updateIndexBuffer(IBObject t_Buffer, void* data, uint64 t_Length);
	void updateTexture(TextureObject t_Tex, Rectangle2D rect, const void* t_Data);

	void drawIndex(TopolgyType topology, uint32 count, uint32 offset = 0,  uint32 base = 0);

	void ClearBuffer(float red, float green, float blue);
	void ClearZBuffer();
	void EndFrame();

	void destroyZBuffer();
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

	PSConstantBuffer PixelShaderCB;
	VSConstantBuffer VertexShaderCB;

	ID3D11Buffer* PixelShaderCBId{nullptr};
	ID3D11Buffer* VertexShaderCBId{nullptr};

	ID3D11BlendState* BlendingStates[BS_Count];

	ShaderObject Shaders[SF_COUNT];

};


template<typename VertexType>
VBObject vertexBufferFactory(Graphics& graphics, asl::TempVector<VertexType>& t_VertexList)
{
	return graphics.createVertexBuffer(sizeof(VertexType), t_VertexList.data(), (uint32)(sizeof(VertexType) * t_VertexList.size()));
}

template<typename IndexType = uint32>
IBObject indexBufferFactory(Graphics& graphics, asl::TempVector<IndexType>& t_IndexList)
{
	return graphics.createIndexBuffer(t_IndexList.data(), (uint32)(sizeof(IndexType) * t_IndexList.size()));
}
