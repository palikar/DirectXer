#include "GraphicsD3D11.hpp"

#include <Config.hpp>

#include <2DVertexShader.hpp>
#include <2DPixelShader.hpp>

#include <PixelShader.hpp>
#include <VertexShader.hpp>

#include <QuadPixelShader.hpp>
#include <QuadVertexShader.hpp>

#include <MTLPixelShader.hpp>
#include <MTLVertexShader.hpp>
#include <MTLInstancedVertexShader.hpp>


#define CSTR_TO_WIDE(cstr, wstr)										\
	int __retval;														\
	retval = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, cstr, -1, NULL, 0); \
	LPWSTR lpWideCharStr = (LPWSTR)alloca(retval * sizeof(WCHAR));		\
	retval = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, cstr, -1, lpWideCharStr, retval)

static DXGI_FORMAT TFToDXGI(TextureFormat format)
{
	switch (format)
	{
	  case TF_RGBA: return DXGI_FORMAT_R8G8B8A8_UNORM;
	  case TF_A: return DXGI_FORMAT_A8_UNORM;
	  case TF_R: return DXGI_FORMAT_R8_UNORM;
	}

	Assert(false, "Usage of unknows texture format.");
	return DXGI_FORMAT_UNKNOWN;
}

static const char* FeatureLevelToString(D3D_FEATURE_LEVEL level)
{
	switch (level) {
	  case D3D_FEATURE_LEVEL_10_1: return "D3D_FEATURE_LEVEL_10_1";
	  case D3D_FEATURE_LEVEL_11_0: return "D3D_FEATURE_LEVEL_11_0";
	  case D3D_FEATURE_LEVEL_11_1: return "D3D_FEATURE_LEVEL_11_1";
	  default: return "Unknow feature level";
	}
}

static inline void SetDebugName(ID3D11DeviceChild* res, String name)
{
	if(res) res->SetPrivateData(WKPDID_D3DDebugObjectName, (uint32)name.size(), name.data());
}

void GraphicsD3D11::InitSwapChain(HWND hWnd, float t_Width, float t_Height)
{
	Textures.reserve(Config::InitialMaxTextures);
	VertexBuffers.reserve(Config::InitialMaxVertexBuffers);
	IndexBuffers.reserve(Config::InitialMaxIndexBuffers);
	ConstantBuffers.reserve(Config::InitialMaxConstantBuffers);

	DXGI_SWAP_CHAIN_DESC1 sd{ 0 };
	sd.BufferCount = 2;
	sd.Width = (uint32)t_Width;
	sd.Height = (uint32)t_Height;
	sd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsd{};
	fsd.RefreshRate.Numerator = 60;
	fsd.RefreshRate.Denominator = 1;
	fsd.Windowed = true;

	IDXGIFactory4* dxgiFactory;
	IDXGISwapChain1* swapChain;
	IDXGIDevice* dxgiDevice;
	IDXGIAdapter* adapter;

	[[maybe_unused]]HRESULT hr;
	GFX_CALL(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));

	UINT initFlags{0};
	if (DebugBuild)
	{
		initFlags |= D3D11_CREATE_DEVICE_DEBUG;
		initFlags |= D3D11_CREATE_DEVICE_PREVENT_INTERNAL_THREADING_OPTIMIZATIONS;
	}

	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	
	GFX_CALL(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, initFlags,
					  featureLevels, 2, D3D11_SDK_VERSION, &Device, nullptr, &Context));

	GFX_CALL(dxgiFactory->CreateSwapChainForHwnd(Device, hWnd, &sd, &fsd, nullptr, &swapChain));
	GFX_CALL(swapChain->QueryInterface(__uuidof(IDXGISwapChain3), (void**)&Swap));
	
	GFX_CALL(Device->QueryInterface(__uuidof(IDXGIDevice), (void**)(&dxgiDevice)));
	
	GFX_CALL(dxgiDevice->GetAdapter(&adapter));
	GFX_CALL(adapter->QueryInterface(__uuidof(IDXGIAdapter3), (void**)(&Adapter)));

	dxgiFactory->Release();
	swapChain->Release();
	adapter->Release();

	GFX_CALL(Context->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), (void**)&Annotator));

	DXDEBUG("[Graphics] Direct3D: {}", FeatureLevelToString(Device->GetFeatureLevel()));
}

void GraphicsD3D11::InitBackBuffer()
{
	ID3D11Resource* pBackBuffer{ nullptr };

	[[maybe_unused]]HRESULT hr;
	GFX_CALL(Swap->GetBuffer(0, __uuidof(ID3D11Resource), (void**)(&pBackBuffer)));
	GFX_CALL(Device->CreateRenderTargetView(pBackBuffer, nullptr, &RenderTargetView));

	SetDebugName(pBackBuffer, "Backbuffer RT");
	SetDebugName(RenderTargetView, "Backbuffer RT");
	
	pBackBuffer->Release();
}

void GraphicsD3D11::InitZBuffer(float width, float height)
{
	[[maybe_unused]]HRESULT hr;

	D3D11_DEPTH_STENCIL_DESC dsDesc{0};
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

	ID3D11DepthStencilState *pDSState;
	GFX_CALL(Device->CreateDepthStencilState(&dsDesc, &pDSState));
	Context->OMSetDepthStencilState(pDSState, 1);

	ID3D11Texture2D *depthStencil;
	D3D11_TEXTURE2D_DESC descDepth{0};
	descDepth.Width = (uint32)width;
	descDepth.Height = (uint32)height;
	descDepth.MipLevels = 1u;
	descDepth.ArraySize = 1u;
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count = 1u;
	descDepth.SampleDesc.Quality = 0u;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	GFX_CALL( Device->CreateTexture2D( &descDepth,nullptr,&depthStencil ) );

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV{0};
	descDSV.Format = DXGI_FORMAT_D32_FLOAT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0u;
	GFX_CALL(Device->CreateDepthStencilView(depthStencil, &descDSV, &DepthStencilView));

	Context->OMSetRenderTargets(1, &RenderTargetView, DepthStencilView);
	Assert(DepthStencilView, "Can't create the depth stencil view for the backbuffer");

	SetDebugName(depthStencil, "Backbuffer DS");
	SetDebugName(DepthStencilView, "Backbuffer DS");
}

void GraphicsD3D11::InitRasterizationsStates()
{
	ID3D11RasterizerState* rastStateNormal;
	ID3D11RasterizerState* rastStateDebug;

	D3D11_RASTERIZER_DESC rastDesc{0};
	rastDesc.CullMode = D3D11_CULL_NONE;
	rastDesc.FillMode = D3D11_FILL_SOLID;
	rastDesc.FrontCounterClockwise = false;
	rastDesc.ScissorEnable  = false;

	[[maybe_unused]]HRESULT hr;
	GFX_CALL(Device->CreateRasterizerState(&rastDesc, &rastStateNormal));

	rastDesc.CullMode = D3D11_CULL_NONE;
	rastDesc.FillMode = D3D11_FILL_WIREFRAME;
	rastDesc.FrontCounterClockwise = false;
	rastDesc.ScissorEnable  = false;
	GFX_CALL(Device->CreateRasterizerState(&rastDesc, &rastStateDebug));

	RasterizationsStates[RS_NORMAL] = rastStateNormal;
	RasterizationsStates[RS_DEBUG] = rastStateDebug;

	SetDebugName(RasterizationsStates[RS_NORMAL], "Normal RS");
	SetDebugName(RasterizationsStates[RS_DEBUG], "Debug RS");

}

void GraphicsD3D11::InitSamplers()
{
	{
		// Linear sampler
		
		D3D11_SAMPLER_DESC desc;
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.MipLODBias = 0.0f;
		desc.MaxAnisotropy = 1;
		desc.ComparisonFunc = D3D11_COMPARISON_NOT_EQUAL;
		desc.BorderColor[0] = 0.0f;
		desc.BorderColor[1] = 0.0f;
		desc.BorderColor[2] = 0.0f;
		desc.BorderColor[3] = 0.0f;
		desc.MinLOD = 0.0f;
		desc.MaxLOD = D3D11_FLOAT32_MAX;

		ID3D11SamplerState* state;
		Device->CreateSamplerState(&desc, &state);

		Context->PSSetSamplers(0, 1, &state);
		Context->VSSetSamplers(0, 1, &state);

		SetDebugName(state, "Linear Sampler");
	}

	{
		// Point sampler
		
		D3D11_SAMPLER_DESC desc;
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.MipLODBias = 0.0f;
		desc.MaxAnisotropy = 1;
		desc.ComparisonFunc = D3D11_COMPARISON_NOT_EQUAL;
		desc.BorderColor[0] = 0.0f;
		desc.BorderColor[1] = 0.0f;
		desc.BorderColor[2] = 0.0f;
		desc.BorderColor[3] = 0.0f;
		desc.MinLOD = 0.0f;
		desc.MaxLOD = D3D11_FLOAT32_MAX;

		ID3D11SamplerState* state;
		Device->CreateSamplerState(&desc, &state);

		Context->PSSetSamplers(1, 1, &state);
		Context->VSSetSamplers(1, 1, &state);

		SetDebugName(state, "Point Sampler");
	}
}

void GraphicsD3D11::InitBlending()
{
	D3D11_BLEND_DESC desc;

	desc.IndependentBlendEnable = FALSE;
	desc.AlphaToCoverageEnable = FALSE;

	// alpha blending
	{
		desc.RenderTarget[0].BlendEnable = TRUE;
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		[[maybe_unused]]HRESULT hr;
		GFX_CALL(Device->CreateBlendState( &desc, &BlendingStates[BS_AlphaBlending]));

		SetDebugName(BlendingStates[BS_AlphaBlending], "Alpha Blending");
	}

	// premultiplied alpha
	{
		desc.RenderTarget[0].BlendEnable = TRUE;
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_DEST_ALPHA;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		[[maybe_unused]]HRESULT hr;
		GFX_CALL(Device->CreateBlendState( &desc, &BlendingStates[BS_PremultipliedAlpha]));

		SetDebugName(BlendingStates[BS_PremultipliedAlpha], "Premultipled Alpha");
	}
}

void GraphicsD3D11::SetBlendingState(BlendingState t_State)
{
	Context->OMSetBlendState(BlendingStates[t_State], NULL, D3D11_COLOR_WRITE_ENABLE_ALL);
}

void GraphicsD3D11::ResizeBackBuffer(float width, float height)
{
	RenderTargetView->Release();
	Swap->ResizeBuffers(0, (uint32)width, (uint32)height, DXGI_FORMAT_UNKNOWN, 0);
	InitBackBuffer();
}

void GraphicsD3D11::DestroyZBuffer()
{
	DepthStencilView->Release();
}

void GraphicsD3D11::Destroy()
{

	Context->Release();
	Swap->Release();
	Device->Release();

	RenderTargetView->Release();
	DepthStencilView->Release();
}

void GraphicsD3D11::EndFrame()
{
	DxProfileCode(DxTimedBlock(Phase_Rendering, "Present"));
	
	HRESULT hr;
	DXGI_PRESENT_PARAMETERS params{0};
	params.DirtyRectsCount = 0;
	if( FAILED( hr = Swap->Present1( 1u, 0u, &params ) ) )
	{
		if( hr == DXGI_ERROR_DEVICE_REMOVED )
		{
			Assert(false, "Lost the rendering device. This is bad and we have to crash");
		}
		else
		{
			DXWARNING("[Graphics] Can't present frame but we'll not crash here");
		}
	}
	Context->OMSetRenderTargets(1, &RenderTargetView, DepthStencilView);
	LastCallsCount = DrawCallsCount;
	DrawCallsCount = 0;
}

void GraphicsD3D11::ClearBuffer(float red, float green, float blue)
{
	const float color[] = { red, green, blue };
	Context->ClearRenderTargetView(RenderTargetView, color);
}

void GraphicsD3D11::ClearZBuffer()
{
	Context->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH | D3D10_CLEAR_STENCIL, 1.0f, 0u);
}

void GraphicsD3D11::ClearRT(RTObject& t_RT)
{
	auto rtv = Textures.at(t_RT.Color).rtv;
	auto dsv = Textures.at(t_RT.DepthStencil).dsv;
	
	const float color[] = { 0.0f, 0.0f, 0.0f };
	Context->ClearRenderTargetView(rtv, color);
	Context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D10_CLEAR_STENCIL, 1.0f, 0u);
}

void GraphicsD3D11::SetRasterizationState(RasterizationState t_State)
{
	Context->RSSetState(RasterizationsStates[t_State]);
}

void GraphicsD3D11::SetScissor(Rectangle2D t_Rect)
{
	D3D11_RECT sissorRect;
	sissorRect.right= (int)(roundf(t_Rect.Position.x));
	sissorRect.top= (int)(roundf(t_Rect.Position.y));
	sissorRect.left = (int)(roundf(t_Rect.Position.x + t_Rect.Size.x));
	sissorRect.bottom = (int)(roundf(t_Rect.Position.y + t_Rect.Size.y));

	Context->RSSetScissorRects(1, &sissorRect);
}

void GraphicsD3D11::SetDepthStencilState(DepthStencilState t_State, uint32 t_RefValue)
{
	Context->OMSetDepthStencilState(DepthStencilStates[t_State], t_RefValue);
}

void GraphicsD3D11::BindPSConstantBuffers(ConstantBufferId t_Id, uint16 t_Slot)
{
	Context->PSSetConstantBuffers(t_Slot, 1, &ConstantBuffers.at(t_Id).id);
}

void GraphicsD3D11::BindVSConstantBuffers(ConstantBufferId t_Id, uint16 t_Slot)
{
	Context->VSSetConstantBuffers(t_Slot, 1, &ConstantBuffers.at(t_Id).id);
}

void GraphicsD3D11::UpdateTexture(TextureId t_Id, Rectangle2D rect, const void* t_Data, int t_Pitch)
{
	D3D11_BOX box;
	box.left = (uint32)(rect.Position.x);
	box.top = (uint32)(rect.Position.y);
	box.right = (uint32)(rect.Position.x + rect.Size.x);
	box.bottom = (uint32)(rect.Position.y + rect.Size.y);
	box.front = 0;
	box.back = 1;

	Context->UpdateSubresource(Textures.at(t_Id).tp, 0, &box, t_Data, (uint32)(rect.Size.x * t_Pitch), 0);
}

bool GraphicsD3D11::CreateTexture(TextureId id, TextureDescription description, const void* t_Data)
{
	TextureObject to;

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = description.Width;
	desc.Height = description.Height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = TFToDXGI( description.Format);
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;

	[[maybe_unused]]HRESULT hr;
	if(t_Data)
	{
		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = t_Data;
		data.SysMemPitch = description.Width*4;
		GFX_CALL(Device->CreateTexture2D(&desc, &data, &to.tp));
	}
	else
	{
		GFX_CALL(Device->CreateTexture2D(&desc, nullptr, &to.tp));
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	GFX_CALL(Device->CreateShaderResourceView(to.tp, &srvDesc, &to.srv));

	[[maybe_unused]] bool res = Textures.insert({ id, to }).second;
	Assert(res, "Usage of non-existaned texture");
	Telemetry::AddMemory(GPURes_Texture, BytesPerPixel(description.Format) * description.Width * description.Height);
	
	return true;
}

bool GraphicsD3D11::CreateDSTexture(TextureId id, TextureDescription description)
{
	TextureObject to;

	D3D11_TEXTURE2D_DESC desc{ 0 };
	desc.Width = description.Width ;
	desc.Height = description.Height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT ;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;	

	[[maybe_unused]] HRESULT hr;
	GFX_CALL(Device->CreateTexture2D(&desc, nullptr, &to.tp));

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{ 0 };

	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = 0;
			
	GFX_CALL(Device->CreateDepthStencilView(to.tp, &dsvDesc, &to.dsv));

	[[maybe_unused]]bool res = Textures.insert({id, to}).second;
	Assert(res, "Usage of non-existaned texture");
	Telemetry::AddMemory(GPURes_Texture, BytesPerPixel(description.Format) * description.Width * description.Height);
	
	return true;
}

void GraphicsD3D11::SetTextureName(TextureId id, String name)
{
	auto obj = Textures.at(id);
	obj.tp->SetPrivateData(WKPDID_D3DDebugObjectName, (uint32)name.size(), name.data());
	obj.srv->SetPrivateData(WKPDID_D3DDebugObjectName, (uint32)name.size(), name.data());
	if (obj.rtv) obj.rtv->SetPrivateData(WKPDID_D3DDebugObjectName, (uint32)name.size(), name.data());
	if (obj.dsv) obj.dsv->SetPrivateData(WKPDID_D3DDebugObjectName, (uint32)name.size(), name.data());
}
void GraphicsD3D11::SetVertexBufferName(VertexBufferId id, String name)
{
	auto obj = VertexBuffers.at(id);
	obj.id->SetPrivateData(WKPDID_D3DDebugObjectName, (uint32)name.size(), name.data());
}

void GraphicsD3D11::SetIndexBufferName(IndexBufferId id, String name)
{
	auto obj = IndexBuffers.at(id);
	obj.id->SetPrivateData(WKPDID_D3DDebugObjectName, (uint32)name.size(), name.data());
}

void GraphicsD3D11::SetConstantBufferName(ConstantBufferId id, String name)
{
	auto obj = ConstantBuffers.at(id);
	obj.id->SetPrivateData(WKPDID_D3DDebugObjectName, (uint32)name.size(), name.data());
}

void GraphicsD3D11::DestroyCubeTexture(TextureId id)
{
	auto tex = Textures.erase_at(id);

	tex.tp->Release();
	if (tex.srv) tex.srv->Release();
	if (tex.rtv) tex.rtv->Release();
	if (tex.dsv) tex.dsv->Release();
}

void GraphicsD3D11::DestroyVertexBuffer(VertexBufferId id)
{
	auto buf = VertexBuffers.erase_at(id);
	buf.id->Release();
}

void GraphicsD3D11::DestroyIndexBuffer(IndexBufferId id)
{
	auto buf = IndexBuffers.erase_at(id);
	buf.id->Release();
}

void GraphicsD3D11::DestroyConstantBuffer(ConstantBufferId id)
{
	auto buf = ConstantBuffers.erase_at(id);
	buf.id->Release();
}

void GraphicsD3D11::SetRenderTarget(RTObject& t_RT)
{
	ID3D11RenderTargetView* rtv = Textures.at(t_RT.Color).rtv;
	auto dsv = Textures.at(t_RT.DepthStencil).dsv;
	Context->OMSetRenderTargets(1, &rtv, dsv);
}

void GraphicsD3D11::ResetRenderTarget()
{
	Context->OMSetRenderTargets(1, &RenderTargetView, DepthStencilView);
}

bool GraphicsD3D11::CreateRenderTexture(TextureId t_Id, RenderTargetDescription description)
{
	TextureObject to;

	D3D11_TEXTURE2D_DESC desc{ 0 };
	desc.Width = description.Width;
	desc.Height = description.Height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = TFToDXGI(description.Format);
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT ;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	[[maybe_unused]] HRESULT hr;
	
	GFX_CALL(Device->CreateTexture2D(&desc, nullptr, &to.tp));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{ 0 };
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	GFX_CALL(Device->CreateShaderResourceView(to.tp, &srvDesc, &to.srv));

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{ 0 };
	rtvDesc.Format = desc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice= 0;

	GFX_CALL(Device->CreateRenderTargetView(to.tp, &rtvDesc, &to.rtv));
	Telemetry::AddMemory(GPURes_Texture, BytesPerPixel(description.Format) * description.Width * description.Height);
	
	return true;
}

bool GraphicsD3D11::CreateCubeTexture(TextureId id, TextureDescription description, void* t_Data[6])
{
	TextureObject to;

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = description.Width;
	desc.Height = description.Height;
	desc.MipLevels = 1;
	desc.ArraySize = 6;
	desc.Format = TFToDXGI(description.Format);
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	D3D11_SUBRESOURCE_DATA data[6];
	for (size_t i = 0; i < 6; ++i)
	{
		data[i].pSysMem = t_Data[i];
		data[i].SysMemPitch = description.Width *4;
	}

	[[maybe_unused]]HRESULT hr;
	GFX_CALL(Device->CreateTexture2D(&desc, data, &to.tp));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	GFX_CALL(Device->CreateShaderResourceView(to.tp, &srvDesc, &to.srv));
	
	[[maybe_unused]]bool res = Textures.insert({id, to }).second;
	Assert(res, "Usage of non-existaned texture");
	Telemetry::AddMemory(GPURes_Texture, BytesPerPixel(description.Format) * description.Width * description.Height * 6);

	return true;

}

bool GraphicsD3D11::CreateVertexBuffer(VertexBufferId id, uint32 structSize, void* data, uint32 dataSize, bool dynamic)
{
	ID3D11Buffer* pVertexBuffer;
	D3D11_BUFFER_DESC vertexBufferDesc{ 0 };
	vertexBufferDesc.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.ByteWidth = dataSize;
	vertexBufferDesc.StructureByteStride = structSize;

	[[maybe_unused]]HRESULT hr;
	if (data)
	{
		D3D11_SUBRESOURCE_DATA bufferData{ 0 };
		bufferData.pSysMem = data;
		GFX_CALL(Device->CreateBuffer(&vertexBufferDesc, &bufferData, &pVertexBuffer));
	}
	else
	{
		GFX_CALL(Device->CreateBuffer(&vertexBufferDesc, nullptr, &pVertexBuffer));
	}

	[[maybe_unused]]bool res = VertexBuffers.insert({id, VBObject{structSize, pVertexBuffer}}).second;
	Assert(res, "Usage of non-existaned vertex buffer");
	Telemetry::AddMemory(GPURes_VertexBuffer, dataSize);

	return true;
}

bool GraphicsD3D11::CreateIndexBuffer(IndexBufferId id, void* data, uint32 dataSize, bool dynamic)
{
	ID3D11Buffer* pIndexBuffer;
	D3D11_BUFFER_DESC indexBufferDesc{ 0 };
	indexBufferDesc.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.ByteWidth = dataSize;
	indexBufferDesc.StructureByteStride = sizeof(uint32);

	[[maybe_unused]]HRESULT hr;
	if(data)
	{
		D3D11_SUBRESOURCE_DATA indexBufferData{0};
		indexBufferData.pSysMem = data;
		GFX_CALL(Device->CreateBuffer(&indexBufferDesc, &indexBufferData, &pIndexBuffer));
	}
	else
	{
		GFX_CALL(Device->CreateBuffer(&indexBufferDesc, nullptr, &pIndexBuffer));
	}

	[[maybe_unused]]bool res = IndexBuffers.insert({ id, IBObject{pIndexBuffer } }).second;
	Assert(res, "Usage of non-existaned index buffer");
	Telemetry::AddMemory(GPURes_IndexBuffer, dataSize);
	
	return true;
}

void GraphicsD3D11::InitResources()
{

	[[maybe_unused]]HRESULT hr;

	{
		// Debug shader
		const D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
			{"Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"Color", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"Texcoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"Normal", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
		
		ShaderObject shaderObject;
		GFX_CALL(Device->CreatePixelShader(g_PixelShader, Size(g_PixelShader), nullptr, &shaderObject.ps));
		GFX_CALL(Device->CreateVertexShader(g_VertexShader, Size(g_VertexShader), nullptr, &shaderObject.vs));
		GFX_CALL(Device->CreateInputLayout(layoutDesc, (uint32)std::size(layoutDesc), g_VertexShader, Size(g_VertexShader), &shaderObject.il));
		Shaders[SF_DEBUG] = shaderObject;

		SetDebugName(shaderObject.ps, "DebugPixelShader");
		SetDebugName(shaderObject.vs, "DebugVertexShader");
		SetDebugName(shaderObject.il, "ColorVertex");
	}

	{
		// 2D shader
		const D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
			{"Position", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"Texcoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"Color", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"Additional", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"Type", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
		
		ShaderObject shaderObject;
		GFX_CALL(Device->CreatePixelShader(g_2DPixelShader, Size(g_2DPixelShader), nullptr, &shaderObject.ps));
		GFX_CALL(Device->CreateVertexShader(g_2DVertexShader, Size(g_2DVertexShader), nullptr, &shaderObject.vs));
		GFX_CALL(Device->CreateInputLayout(layoutDesc, (uint32)std::size(layoutDesc), g_2DVertexShader, Size(g_2DVertexShader), &shaderObject.il));
		Shaders[SF_2D] = shaderObject;

		SetDebugName(shaderObject.ps, "2DPixelShader");
		SetDebugName(shaderObject.vs, "2DVertexShader");
		SetDebugName(shaderObject.il, "Vertex2D");
		
	}

	{
		// Quad shader
		
		ShaderObject shaderObject;
		GFX_CALL(Device->CreatePixelShader(g_QuadPixelShader, Size(g_QuadPixelShader), nullptr, &shaderObject.ps));
		GFX_CALL(Device->CreateVertexShader(g_QuadVertexShader, Size(g_QuadVertexShader), nullptr, &shaderObject.vs));
		shaderObject.il = nullptr;
		Shaders[SF_QUAD] = shaderObject;
	}

	{
		// MTL shader
		const D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
			{"Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"Texcoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"Normal", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
		
		ShaderObject shaderObject;
		GFX_CALL(Device->CreatePixelShader(g_MTLPixelShader, Size(g_MTLPixelShader), nullptr, &shaderObject.ps));
		GFX_CALL(Device->CreateVertexShader(g_MTLVertexShader, Size(g_MTLVertexShader), nullptr, &shaderObject.vs));
		GFX_CALL(Device->CreateInputLayout(layoutDesc, (uint32)std::size(layoutDesc), g_MTLVertexShader, Size(g_MTLVertexShader), &shaderObject.il));
		Shaders[SF_MTL] = shaderObject;

		SetDebugName(shaderObject.ps, "MtlPixelShader");
		SetDebugName(shaderObject.vs, "MtlVertexShader");
		SetDebugName(shaderObject.il, "MtlVertex");
	}

	{
		// MTL Instanced shader
		const D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
			{"Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"Texcoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"Normal", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0},

			{"MODEL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"MODEL", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"MODEL", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"MODEL", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1},

			{"INVMODEL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64 + 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"INVMODEL", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64 + 16, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"INVMODEL", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64 + 32, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"INVMODEL", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64 + 48, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		};
		
		ShaderObject shaderObject;
		shaderObject.ps = Shaders[SF_MTL].ps;
		GFX_CALL(Device->CreateVertexShader(g_MTLInstancedVertexShader, Size(g_MTLInstancedVertexShader), nullptr, &shaderObject.vs));
		GFX_CALL(Device->CreateInputLayout(layoutDesc, (uint32)std::size(layoutDesc), g_MTLInstancedVertexShader, Size(g_MTLInstancedVertexShader), &shaderObject.il));
		Shaders[SF_MTLInst] = shaderObject;

		SetDebugName(shaderObject.ps, "MtlInstancedPixelShader");
		SetDebugName(shaderObject.vs, "MtlInstancedVertexShader");
		SetDebugName(shaderObject.il, "MtlVertexInstanced");
	}


		
	D3D11_BUFFER_DESC vertexShaderCBDesc{ 0 };
	vertexShaderCBDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexShaderCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	vertexShaderCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexShaderCBDesc.MiscFlags = 0;
	vertexShaderCBDesc.ByteWidth = sizeof(VSConstantBuffer);
	vertexShaderCBDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertexShaderCBData{0};
	vertexShaderCBData.pSysMem = &VertexShaderCB;
	GFX_CALL(Device->CreateBuffer(&vertexShaderCBDesc, &vertexShaderCBData, &VertexShaderCBId));

	SetDebugName(VertexShaderCBId, "Primary VS Constant Buffer");

	Context->VSSetConstantBuffers(0, 1, &VertexShaderCBId);


	D3D11_BUFFER_DESC pixelShaderCBDesc{ 0 };
	pixelShaderCBDesc.Usage = D3D11_USAGE_DYNAMIC;
	pixelShaderCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	pixelShaderCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pixelShaderCBDesc.MiscFlags = 0;
	pixelShaderCBDesc.ByteWidth = sizeof(PSConstantBuffer);
	pixelShaderCBDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA pixelShaderCBData{0};
	pixelShaderCBData.pSysMem = &PixelShaderCB;
	GFX_CALL(Device->CreateBuffer(&pixelShaderCBDesc, &pixelShaderCBData, &PixelShaderCBId));
	Context->PSSetConstantBuffers(0, 1, &PixelShaderCBId);

	SetDebugName(PixelShaderCBId, "Primary PS Constant Buffer");

}

void GraphicsD3D11::InitDepthStencilStates()
{
	{
		// Normal
	
		D3D11_DEPTH_STENCIL_DESC dsDesc{0};
		dsDesc.DepthEnable = TRUE;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

		dsDesc.StencilEnable = FALSE;
		dsDesc.StencilReadMask = 0x00;
		dsDesc.StencilWriteMask = 0x00;

		dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	
		[[maybe_unused]]HRESULT hr;
		GFX_CALL(Device->CreateDepthStencilState(&dsDesc, &DepthStencilStates[DSS_Normal]));

		SetDebugName(DepthStencilStates[DSS_Normal], "Normal DS");
	}

	{
		// 2D Rendering
	
		D3D11_DEPTH_STENCIL_DESC dsDesc{0};
		dsDesc.DepthEnable = TRUE;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dsDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;

		dsDesc.StencilEnable = FALSE;
		dsDesc.StencilReadMask = 0x00;
		dsDesc.StencilWriteMask = 0x00;

		dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	
		[[maybe_unused]]HRESULT hr;
		GFX_CALL(Device->CreateDepthStencilState(&dsDesc, &DepthStencilStates[DSS_2DRendering]));

		SetDebugName(DepthStencilStates[DSS_2DRendering], "2D Rendering DS");
	}
}

bool GraphicsD3D11::CreateConstantBuffer(ConstantBufferId id, uint32 t_Size, void* t_InitData)
{
	CBObject cb;

	D3D11_BUFFER_DESC desc{ 0 };
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.ByteWidth = t_Size;
	desc.StructureByteStride = 0;

	[[maybe_unused]]HRESULT hr;
	if (t_InitData)
	{
		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = t_InitData;
		GFX_CALL(Device->CreateBuffer(&desc, &data, &cb.id));
	}
	else
	{
		GFX_CALL(Device->CreateBuffer(&desc, nullptr, &cb.id));
	}

	[[maybe_unused]]bool res = ConstantBuffers.insert({id, cb}).second;
	Assert(res, "Usage of non-existaned constant buffer");
	Telemetry::AddMemory(GPURes_ConstantBuffer, t_Size);
	return true;
}

void GraphicsD3D11::UpdateCBs()
{
	D3D11_MAPPED_SUBRESOURCE msr;

	Context->Map(VertexShaderCBId, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	memcpy(msr.pData, &VertexShaderCB, sizeof(VertexShaderCB));
	Context->Unmap(VertexShaderCBId, 0);

	Context->Map(PixelShaderCBId, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	memcpy(msr.pData, &PixelShaderCB, sizeof(PixelShaderCB));
	Context->Unmap(PixelShaderCBId, 0);

}

void GraphicsD3D11::UpdateCBs(ConstantBufferId& t_Id, uint32 t_Length, void* t_Data)
{
	auto id = ConstantBuffers.at(t_Id).id;

	D3D11_MAPPED_SUBRESOURCE msr;
	[[maybe_unused]]HRESULT hr;
	GFX_CALL(Context->Map(id, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr));
	memcpy(msr.pData, t_Data, t_Length);
	Context->Unmap(id, 0);
}

void GraphicsD3D11::UpdateVertexBuffer(VertexBufferId t_Id, void* data, uint64 t_Length)
{
	auto id = VertexBuffers.at(t_Id).id;
	
	D3D11_MAPPED_SUBRESOURCE msr;
	[[maybe_unused]]HRESULT hr;
	GFX_CALL(Context->Map(id, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr));
	memcpy(msr.pData, data, t_Length);
	Context->Unmap(id, 0);
}

void GraphicsD3D11::UpdateIndexBuffer(IndexBufferId t_Id, void* data, uint64 t_Length)
{
	auto id = IndexBuffers.at(t_Id).id;
	D3D11_MAPPED_SUBRESOURCE msr;
	[[maybe_unused]]HRESULT hr;
	GFX_CALL(Context->Map(id, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr));
	memcpy(msr.pData, data, t_Length);
	Context->Unmap(id, 0);
}

void GraphicsD3D11::BindTexture(uint32 t_Slot, TextureId t_Id)
{
	Context->PSSetShaderResources(t_Slot, 1, &Textures.at(t_Id).srv);
}

void GraphicsD3D11::BindVSTexture(uint32 t_Slot, TextureId t_Id)
{
	Context->VSSetShaderResources(t_Slot, 1, &Textures.at(t_Id).srv);
}

void GraphicsD3D11::SetViewport(float x, float y, float width, float height)
{
	D3D11_VIEWPORT vp;
	vp.Width = width;
	vp.Height = height;
	vp.MinDepth = 0;
	vp.MaxDepth = 1;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;

	Context->RSSetViewports(1, &vp);
}

void GraphicsD3D11::SetShaderConfiguration(ShaderConfig t_Config)
{
	uint8 shaderObjectIndex = 0xFF & t_Config;
	uint8 shaderType = (0xFF00 & t_Config) >> 8;

	auto& shaderObject = Shaders[shaderObjectIndex];

	Context->PSSetShader(shaderObject.ps, nullptr, 0);
	Context->VSSetShader(shaderObject.vs, nullptr, 0);
	Context->IASetInputLayout(shaderObject.il);

	PixelShaderCB.shaderType = shaderType;
	VertexShaderCB.shaderType = shaderType;
}

void GraphicsD3D11::BindIndexBuffer(IndexBufferId id)
{
	Context->IASetIndexBuffer(IndexBuffers.at(id).id, DXGI_FORMAT_R32_UINT, 0);
}

void GraphicsD3D11::BindVertexBuffer(VertexBufferId t_Id, uint32 offset, uint32 slot)
{
	const auto& buffer = VertexBuffers.at(t_Id);
	Context->IASetVertexBuffers(slot, 1, &buffer.id, &buffer.structSize, &offset);
}

void GraphicsD3D11::DrawIndex(TopolgyType topology, uint32 count, uint32 offset, uint32 base)
{

	// @Todo: Make this in a function of its own

	uint32 factor = 1;
	switch (topology) {
	  case TT_TRIANGLES:
		  Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		  break;
	  case TT_LINES:
		  Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		  factor = 1;
		  break;
	}

	Context->DrawIndexed(count/factor, offset, base);
	DrawCallsCount += 1;
}

void GraphicsD3D11::Draw(TopolgyType topology, uint32 count, uint32 base)
{
	switch (topology) {
	  case TT_TRIANGLES:
		  Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		  break;
	  case TT_TRIANGLE_STRIP:
		  Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		  break;
	  case TT_LINES:
		  Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		  break;
	}

	Context->Draw(count, base);
	DrawCallsCount += 1;
}

void GraphicsD3D11::DrawInstancedIndex(TopolgyType topology, uint32 count, uint32 instances, uint32 offset, uint32 base)
{
	uint32 factor = 1;
	switch (topology) {
	  case TT_TRIANGLES:
		  Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		  break;
	  case TT_LINES:
		  Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		  factor = 2;
		  break;
	}

	Context->DrawIndexedInstanced(count/factor, instances, offset, base, 0);
	DrawCallsCount += 1;
}

GPUMemoryReport GraphicsD3D11::ReportMemory()
{
	DXGI_QUERY_VIDEO_MEMORY_INFO info;
	Adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &info);
	GPUMemoryReport report;
	report.Budget = info.Budget;
	report.Usage = info.CurrentUsage;
	return report;
}

void GraphicsD3D11::MakeMarker(const char* name)
{
	int retval;
	retval = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, name, -1, NULL, 0);

	LPWSTR lpWideCharStr = (LPWSTR)alloca(retval * sizeof(WCHAR));
	retval = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, name, -1, lpWideCharStr, retval);

	Annotator->SetMarker(lpWideCharStr);
}

void GraphicsD3D11::PushMarker(const char* name)
{
	int retval;
	retval = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, name, -1, NULL, 0);

	LPWSTR lpWideCharStr = (LPWSTR)alloca(retval * sizeof(WCHAR));
	retval = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, name, -1, lpWideCharStr, retval);

	Annotator->BeginEvent(lpWideCharStr);
}

void GraphicsD3D11::PopMarker()
{
	Annotator->EndEvent();
}

void GraphicsD3D11::BeginTimingQuery()
{
	if (DisjointTimestampQuery) return;
	
	D3D11_QUERY_DESC desc{ 0 };
	desc.MiscFlags = 0;

	[[maybe_unused]] HRESULT hr;
	desc.Query =  D3D11_QUERY_TIMESTAMP_DISJOINT;
	GFX_CALL(Device->CreateQuery(&desc, &DisjointTimestampQuery));
	
	desc.Query =  D3D11_QUERY_TIMESTAMP;
	GFX_CALL(Device->CreateQuery(&desc, &BeginTimeQuery));
	GFX_CALL(Device->CreateQuery(&desc, &EndTimeQuery));

	Context->Begin(DisjointTimestampQuery);
	Context->End(BeginTimeQuery);

	SetDebugName(DisjointTimestampQuery, "DisjointTimeQuery");
	SetDebugName(BeginTimeQuery, "BeginTimeQuery");
	SetDebugName(EndTimeQuery, "EndTimeQuery");
}

void GraphicsD3D11::EndTimingQuery()
{
	Context->End(EndTimeQuery);
	Context->End(DisjointTimestampQuery);
}

bool GraphicsD3D11::GetTimingResult(GPUTimingResult& result)
{
	//result.Time = 0.0f;
	if (!DisjointTimestampQuery) return true;

	if (Context->GetData(DisjointTimestampQuery, NULL, 0, 0) != S_OK) return false;

	D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointData;
	if (Context->GetData(DisjointTimestampQuery, &disjointData, sizeof(D3D11_QUERY_DATA_TIMESTAMP_DISJOINT), 0) != S_OK) return false;

	if (disjointData.Disjoint) return false;

	UINT64 beginTime;
	if (Context->GetData(BeginTimeQuery, &beginTime, sizeof(UINT64), 0) != S_OK) return false;
		
	UINT64 endTime;
	if (Context->GetData(EndTimeQuery, &endTime, sizeof(UINT64), 0) != S_OK) return false;

	result.Time = float(endTime - beginTime) / float(disjointData.Frequency) * 1000.0f;

	BeginTimeQuery->Release();
	EndTimeQuery->Release();
	DisjointTimestampQuery->Release();

	BeginTimeQuery = nullptr;
	EndTimeQuery = nullptr;
	DisjointTimestampQuery = nullptr;
	
	return true;
}

void GraphicsD3D11::BeginStatisticsQuery()
{
	if (StatisticsQuery) return;
	
	D3D11_QUERY_DESC desc{ 0 };
	desc.MiscFlags = 0;

	[[maybe_unused]] HRESULT hr;
	desc.Query =  D3D11_QUERY_PIPELINE_STATISTICS;
	GFX_CALL(Device->CreateQuery(&desc, &StatisticsQuery));

	Context->Begin(StatisticsQuery);
	
	SetDebugName(StatisticsQuery, "StatsQuery");
}

void GraphicsD3D11::EndStatisticsQuery()
{
	Context->End(StatisticsQuery);
}

bool GraphicsD3D11::GetStatisticsResult(GPUStatsResult& result)
{
	if (!StatisticsQuery) return true;

	D3D11_QUERY_DATA_PIPELINE_STATISTICS stats;
	if (Context->GetData(StatisticsQuery, &stats, sizeof(D3D11_QUERY_DATA_PIPELINE_STATISTICS), 0) != S_OK) return false;

	result.VerticesCount = stats.IAVertices;
	result.PrimitivesCount = stats.IAPrimitives;
	result.VSInvocationsCount = stats.VSInvocations;
	result.PSInvocationsCount = stats.PSInvocations;

	StatisticsQuery->Release();
	StatisticsQuery = nullptr;
	return true;
	
}
