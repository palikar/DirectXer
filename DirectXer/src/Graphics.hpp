#pragma once

#include "PlatformWindows\IncludeWin.hpp"
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

struct VBObject
{
	uint32 structSize;
	ID3D11Buffer* id{nullptr};
};

struct IBObject
{
	ID3D11Buffer* id{nullptr};
};

struct CBObject
{
	ID3D11Buffer* id{ nullptr };
};

struct TextureObject
{
	ID3D11Texture2D* tp{nullptr};
	ID3D11ShaderResourceView* srv{nullptr};
	ID3D11RenderTargetView* rtv{nullptr};
};

struct DepthStencilTextureObject
{
	ID3D11Texture2D* tp{nullptr};
	ID3D11DepthStencilView* dsv;
};

struct RTObject
{
	TextureObject ColorAttachment;
	DepthStencilTextureObject DepthAttachment;
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

	using VertexBufferType = VBObject;
	using IndexBufferType = IBObject;
	using ConstantBufferType = CBObject;
	using TextureType = TextureObject;
	using ShaderType = ShaderObject;
	using RenderTargetType = RTObject;

	void InitSwapChain(HWND hWnd, float t_Width, float t_Height);
	void InitBackBuffer();
	void InitZBuffer(float width, float height);
	void InitResources();
	void InitRasterizationsStates();
	void InitSamplers();
	void InitBlending();
	void InitDepthStencilStates();

	void ResizeBackBuffer(float width, float height);

	void BindTexture(uint32 t_Slot, TextureObject t_Texture);
	void BindPSConstantBuffers(CBObject* t_Buffers, uint16 t_Count, uint16 t_StartSlot);
	void BindVSConstantBuffers(CBObject* t_Buffers, uint16 t_Count, uint16 t_StartSlot);

	void SetScissor(Rectangle2D t_Rect);
	void SetRasterizationState(RasterizationState t_State = RS_DEBUG);
	void SetDepthStencilState(DepthStencilState t_State = DSS_Normal, uint32 t_RefValue = 0);
	void SetVertexBuffer(VBObject t_buffer, uint32 offset = 0);
	void SetIndexBuffer(IBObject t_buffer);
	void SetViewport(float x, float y, float width, float height);
	void SetShaderConfiguration(ShaderConfig t_Confing);
	void SetBlendingState(BlendingState t_State);
	void SetRenderTarget(RTObject& t_RT);
	void ResetRenderTarget();


	TextureObject CreateTexture(uint16 t_Width, uint16 t_Height, TextureFormat t_Format, const void* t_Data, uint64 t_Length);
	TextureObject CreateCubeTexture(uint16 t_Width, uint16 t_Height, TextureFormat t_Format, void* t_Data[6]);
	VBObject CreateVertexBuffer(uint32 structSize, void* data, uint32 dataSize, bool dynamic = false);
	IBObject CreateIndexBuffer(void* data, uint32 dataSize, bool dynamic = false);
	CBObject CreateConstantBuffer(uint32 t_Size, void* t_InitData);
	RTObject CreateRenderTarget(uint16 t_Width, uint16 t_Height, TextureFormat t_Format, bool needsDS = true);

	void UpdateCBs();
	void UpdateCBs(CBObject& t_CbObject, uint32 t_Length, void* t_Data);
	void UpdateVertexBuffer(VBObject t_Buffer, void* data, uint64 t_Length);
	void UpdateIndexBuffer(IBObject t_Buffer, void* data, uint64 t_Length);
	void UpdateTexture(TextureObject t_Tex, Rectangle2D rect, const void* t_Data, int t_Pitch = 4);

	void DrawIndex(TopolgyType topology, uint32 count, uint32 offset = 0,  uint32 base = 0);
	void Draw(TopolgyType topology, uint32 count, uint32 base);

	void ClearBuffer(float red, float green, float blue);
	void ClearZBuffer();
	void ClearRT(RTObject& t_RT);
	void EndFrame();

	void DestroyZBuffer();
	void Destroy();

  public:
	// @Note: We should probably have a maximum of 8 pointers here (1 cahce line)
	// not sure which data has to be inlined though; what is that we use most commonly
	// together?
	ID3D11Device* Device{ nullptr };
	IDXGISwapChain* Swap{ nullptr };
	ID3D11DeviceContext* Context{ nullptr };

	// @Note: These are the views for the backbuffer
	ID3D11RenderTargetView* RenderTargetView{ nullptr };
	ID3D11DepthStencilView* DepthStencilView{ nullptr };

	ID3D11RasterizerState* RasterizationsStates[RS_COUNT];
	ID3D11BlendState* BlendingStates[BS_Count];

	PSConstantBuffer PixelShaderCB;
	VSConstantBuffer VertexShaderCB;

	ID3D11Buffer* PixelShaderCBId{nullptr};
	ID3D11Buffer* VertexShaderCBId{nullptr};

	ID3D11DepthStencilState* DepthStencilStates[DSS_Count];

	ShaderObject Shaders[SF_COUNT];

};

template<typename VertexType>
VBObject vertexBufferFactory(Graphics& graphics, TempVector<VertexType>& t_VertexList)
{
	return graphics.CreateVertexBuffer(sizeof(VertexType), t_VertexList.data(), (uint32)(sizeof(VertexType) * t_VertexList.size()));
}

template<typename IndexType = uint32>
IBObject indexBufferFactory(Graphics& graphics, TempVector<IndexType>& t_IndexList)
{
	return graphics.CreateIndexBuffer(t_IndexList.data(), (uint32)(sizeof(IndexType) * t_IndexList.size()));
}

void DrawFullscreenQuad(Graphics* Graphics, TextureObject texture, ShaderConfig type);
