#pragma once

#include <Logging.hpp>
#include <Memory.hpp>
#include <Math.hpp>
#include <Utils.hpp>

#include <d3d11.h>
#include <wrl.h>

#include <vector>
#include <array>
#include <cmath>
#include <sstream>

#include <d3dcompiler.h>
#include <DirectXMath.h>

#include <dxerr.h>

#include <GraphicsCommon.hpp>
#include <Types.hpp>


#include "PlatformWindows\IncludeWin.hpp"

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
	ID3D11DepthStencilView* dsv{nullptr};
};

struct ShaderObject
{
	ID3D11InputLayout* il{nullptr};
	ID3D11VertexShader* vs{nullptr};
	ID3D11PixelShader* ps{nullptr};
};

class GraphicsD3D11
{
  public:

	using VertexBufferType = VBObject;
	using IndexBufferType = IBObject;
	using ConstantBufferType = CBObject;
	using TextureType = TextureObject;
	using ShaderType = ShaderObject;

	void InitSwapChain(HWND hWnd, float t_Width, float t_Height);
	void InitBackBuffer();
	void InitZBuffer(float width, float height);
	void InitResources();
	void InitRasterizationsStates();
	void InitSamplers();
	void InitBlending();
	void InitDepthStencilStates();
	
	void ResizeBackBuffer(float width, float height);

	void BindTexture(uint32 t_Slot, TextureId t_Id);
	void BindPSConstantBuffers(ConstantBufferId t_Id, uint16 t_Slot);
	void BindVSConstantBuffers(ConstantBufferId t_Id, uint16 t_Slot);
	void BindVertexBuffer(VertexBufferId t_Id, uint32 offset = 0);
	void BindIndexBuffer(IndexBufferId id);

	void SetScissor(Rectangle2D t_Rect);
	void SetRasterizationState(RasterizationState t_State = RS_DEBUG);
	void SetDepthStencilState(DepthStencilState t_State = DSS_Normal, uint32 t_RefValue = 0);
	void SetViewport(float x, float y, float width, float height);
	void SetShaderConfiguration(ShaderConfig t_Confing);
	void SetBlendingState(BlendingState t_State);
	void SetRenderTarget(RTObject& t_RT);
	void ResetRenderTarget();

	bool CreateTexture(TextureId id, TextureDescription description, const void* t_Data);
	bool CreateDSTexture(TextureId id, TextureDescription description);
	bool CreateRenderTexture(TextureId t_Id, RenderTargetDescription description);
	bool CreateCubeTexture(TextureId id, TextureDescription description, void* t_Data[6]);
	
	bool CreateVertexBuffer(VertexBufferId id, uint32 structSize, void* data, uint32 dataSize, bool dynamic = false);
	bool CreateIndexBuffer(IndexBufferId id, void* data, uint32 dataSize, bool dynamic = false);
	bool CreateConstantBuffer(ConstantBufferId id, uint32 t_Size, void* t_InitData);

	void UpdateCBs();
	void UpdateCBs(ConstantBufferId& t_Id, uint32 t_Length, void* t_Data);
	void UpdateVertexBuffer(VertexBufferId t_Id, void* data, uint64 t_Length);
	void UpdateIndexBuffer(IndexBufferId t_Id, void* data, uint64 t_Length);
	void UpdateTexture(TextureId t_Id, Rectangle2D rect, const void* t_Data, int t_Pitch = 4);

	void DrawIndex(TopolgyType topology, uint32 count, uint32 offset = 0,  uint32 base = 0);
	void Draw(TopolgyType topology, uint32 count, uint32 base);

	void ClearBuffer(float red, float green, float blue);
	void ClearZBuffer();
	void ClearRT(RTObject& t_RT);
	void EndFrame();

	void DestroyZBuffer();
	void Destroy();

  public:
	ID3D11Device* Device{ nullptr };
	IDXGISwapChain* Swap{ nullptr };
	ID3D11DeviceContext* Context{ nullptr };

	// @Note: These are the views for the backbuffer
	ID3D11RenderTargetView* RenderTargetView{ nullptr };
	ID3D11DepthStencilView* DepthStencilView{ nullptr };

	// @Note: Pipeline state objects
	ID3D11RasterizerState* RasterizationsStates[RS_COUNT];
	ID3D11BlendState* BlendingStates[BS_Count];
	ID3D11DepthStencilState* DepthStencilStates[DSS_Count];
	ShaderObject Shaders[SF_COUNT];

	// @Note: These are the primary VS and PX constant buffers; these are the
	// first constant buffers in each shader
	ID3D11Buffer* PixelShaderCBId{nullptr};
	ID3D11Buffer* VertexShaderCBId{nullptr};
	PSConstantBuffer PixelShaderCB;
	VSConstantBuffer VertexShaderCB;

	// @Note: GPU resources
	GPUResourceMap<TextureId, TextureObject> Textures;
	GPUResourceMap<VertexBufferId, VBObject> VertexBuffers;
	GPUResourceMap<IndexBufferId, IBObject> IndexBuffers;
	GPUResourceMap<ConstantBufferId, CBObject> ConstantBuffers;
};

