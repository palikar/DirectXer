#include "Graphics.hpp"


static DXGI_FORMAT TFToDXGI(TextureFormat format)
{
	switch (format)
	{
	  case TF_RGBA: return DXGI_FORMAT_R8G8B8A8_UNORM;
	  case TF_A: return DXGI_FORMAT_A8_UNORM;
	  case TF_R: return DXGI_FORMAT_R8_UNORM;
	}

	assert(false);
	return DXGI_FORMAT_UNKNOWN;
}

void Graphics::initSwapChain(HWND hWnd, float t_Width, float t_Height)
{

	DXGI_SWAP_CHAIN_DESC sd{ 0 };
	sd.BufferDesc.Width = (uint32)t_Width;
	sd.BufferDesc.Height = (uint32)t_Height;
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 0;
	sd.BufferDesc.RefreshRate.Denominator = 0;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = hWnd;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;


	[[maybe_unused]]HRESULT hr;
	GFX_CALL(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
	D3D11_CREATE_DEVICE_DEBUG, nullptr, 0, D3D11_SDK_VERSION, &sd, &Swap, &Device, nullptr, &Context));

}

void Graphics::initBackBuffer()
{

	ID3D11Resource* pBackBuffer{ nullptr };

	[[maybe_unused]]HRESULT hr;
	GFX_CALL(Swap->GetBuffer(0, __uuidof(ID3D11Resource), (void**)(&pBackBuffer)));
	GFX_CALL(Device->CreateRenderTargetView(pBackBuffer, nullptr, &RenderTargetView));
	pBackBuffer->Release();
}

void Graphics::initZBuffer(float width, float height)
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
	assert(DepthStencilView);

}

void Graphics::initRasterizationsStates()
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

}

void Graphics::initSamplers()
{
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
}

void Graphics::initBlending()
{
	D3D11_BLEND_DESC desc;

	desc.IndependentBlendEnable = FALSE;
	desc.AlphaToCoverageEnable = FALSE;

	// alpha blending
	{
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		[[maybe_unused]]HRESULT hr;
		GFX_CALL(Device->CreateBlendState( &desc, &BlendingStates[BS_AlphaBlending]));
	}

	// premultiplied alpha
	{
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_DEST_ALPHA;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		[[maybe_unused]]HRESULT hr;
		GFX_CALL(Device->CreateBlendState( &desc, &BlendingStates[BS_PremultipliedAlpha]));
	}

}

void Graphics::setBlendingState(BlendingState t_State)
{
	Context->OMSetBlendState(BlendingStates[t_State], NULL, D3D11_COLOR_WRITE_ENABLE_ALL);
}

void Graphics::resizeBackBuffer(float width, float height)
{

	RenderTargetView->Release();
	Swap->ResizeBuffers(0, (uint32)width, (uint32)height, DXGI_FORMAT_UNKNOWN, 0);
	initBackBuffer();
}

void Graphics::destroyZBuffer()
{
	DepthStencilView->Release();
}

void Graphics::Destroy()
{

	Context->Release();
	Swap->Release();
	Device->Release();

	RenderTargetView->Release();
	DepthStencilView->Release();
}

void Graphics::EndFrame()
{
	[[maybe_unused]]HRESULT hr;
	if( FAILED( hr = Swap->Present( 1u,0u ) ) )
	{
		if( hr == DXGI_ERROR_DEVICE_REMOVED )
		{
			assert(false);
		}
		else
		{
			assert(false);
		}
	}
}

void Graphics::ClearBuffer(float red, float green, float blue)
{
	const float color[] = { red, green, blue };

	Context->ClearRenderTargetView(RenderTargetView, color);
}

void Graphics::ClearZBuffer()
{
	Context->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH | D3D10_CLEAR_STENCIL, 1.0f, 0u);
}

void Graphics::setRasterizationState(RasterizationState t_State)
{
	Context->RSSetState(RasterizationsStates[t_State]);
}

void Graphics::setScissor(Rectangle2D t_Rect)
{
	D3D11_RECT sissorRect;
	sissorRect.right= (int)(roundf(t_Rect.Position.x));
	sissorRect.top= (int)(roundf(t_Rect.Position.y));
	sissorRect.left = (int)(roundf(t_Rect.Position.x + t_Rect.Size.x));
	sissorRect.bottom = (int)(roundf(t_Rect.Position.y + t_Rect.Size.y));

	Context->RSSetScissorRects(1, &sissorRect);
}

void Graphics::setDepthStencilState(DepthStencilState t_State, uint32 t_RefValue)
{
	Context->OMSetDepthStencilState(DepthStencilStates[t_State], t_RefValue);
}

void Graphics::bindPSConstantBuffers(CBObject* t_Buffers, uint16 t_Count, uint16 t_StartSlot)
{
	Context->PSSetConstantBuffers(t_StartSlot, t_Count, &t_Buffers->id);
}

void Graphics::bindVSConstantBuffers(CBObject* t_Buffers, uint16 t_Count, uint16 t_StartSlot)
{
	Context->VSSetConstantBuffers(t_StartSlot, t_Count, &t_Buffers->id);
}

void Graphics::updateTexture(TextureObject t_Tex, Rectangle2D rect, const void* t_Data, int t_Pitch)
{
	D3D11_BOX box;
	box.left = (uint32)(rect.Position.x);
	box.top = (uint32)(rect.Position.y);
	box.right = (uint32)(rect.Position.x + rect.Size.x);
	box.bottom = (uint32)(rect.Position.y + rect.Size.y);
	box.front = 0;
	box.back = 1;

	Context->UpdateSubresource(t_Tex.tp, 0, &box, t_Data, (uint32)(rect.Size.x * t_Pitch), 0);
}

TextureObject Graphics::createTexture(uint16 t_Width, uint16 t_Height, TextureFormat t_Format, const void* t_Data, uint64 t_Length)
{
	TextureObject to;

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = t_Width;
	desc.Height = t_Height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = TFToDXGI(t_Format);
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
		data.SysMemPitch = t_Width*4;
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

	return to;
}

void Graphics::setRenderTarget(RTObject t_RT)
{
	Context->OMSetRenderTargets(1, &t_RT.ColorAttachment.rtv, t_RT.DepthAttachment.dsv);
}

void Graphics::resetRenderTarget()
{
	Context->OMSetRenderTargets(1, &RenderTargetView, DepthStencilView);
}

RTObject Graphics::createRenderTarget(uint16 t_Width, uint16 t_Height, TextureFormat t_Format, bool needsDS)
{
	RTObject rt;

	// Color attachment creation
	
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = t_Width;
	desc.Height = t_Height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = TFToDXGI(t_Format);
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT ;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	[[maybe_unused]]HRESULT hr;
	
	GFX_CALL(Device->CreateTexture2D(&desc, nullptr, &rt.ColorAttachment.tp));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	GFX_CALL(Device->CreateShaderResourceView(rt.ColorAttachment.tp, &srvDesc, &rt.ColorAttachment.srv));

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = desc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice= 0;

	GFX_CALL(Device->CreateRenderTargetView(rt.ColorAttachment.tp, &rtvDesc, &rt.ColorAttachment.rtv));

	if (needsDS)
	{
		// Depth attachment creation
		desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

		GFX_CALL(Device->CreateTexture2D(&desc, nullptr, &rt.DepthAttachment.tp));

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Flags = 0;
	
		GFX_CALL(Device->CreateDepthStencilView(rt.DepthAttachment.tp, &dsvDesc, &rt.DepthAttachment.dsv));
	}

	return rt;
}

TextureObject Graphics::createCubeTexture(uint16 t_Width, uint16 t_Height, TextureFormat t_Format, void* t_Data[6])
{

	TextureObject to;

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = t_Width;
	desc.Height = t_Height;
	desc.MipLevels = 1;
	desc.ArraySize = 6;
	desc.Format = TFToDXGI(t_Format);
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
		data[i].SysMemPitch = t_Width*4;
	}

	[[maybe_unused]]HRESULT hr;
	GFX_CALL(Device->CreateTexture2D(&desc, data, &to.tp));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	GFX_CALL(Device->CreateShaderResourceView(to.tp, &srvDesc, &to.srv));

	return to;

}

VBObject Graphics::createVertexBuffer(uint32 structSize, void* data, uint32 dataSize,  bool dynamic)
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


	return {structSize, pVertexBuffer};
}

IBObject Graphics::createIndexBuffer(void* data, uint32 dataSize, bool dynamic)
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

	return {pIndexBuffer};
}

void Graphics::initResources()
{

	[[maybe_unused]]HRESULT hr;

	{
		// Debug shader

		ID3DBlob* pBlob;
		ShaderObject shaderObject;
		D3DReadFileToBlob(L"PixelShader.cso", &pBlob);
		GFX_CALL(Device->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &shaderObject.ps));

		ID3D11VertexShader* pVertexShader{0};
		D3DReadFileToBlob(L"VertexShader.cso", &pBlob);
		GFX_CALL(Device->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &shaderObject.vs));

		const D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
			{"Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"Color", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"Texcoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"Normal", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
		GFX_CALL(Device->CreateInputLayout(layoutDesc, (uint32)std::size(layoutDesc), pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &shaderObject.il));

		Shaders[SF_DEBUG] = shaderObject;
	}

	{
		// 2D shader

		ID3DBlob* pBlob;
		ShaderObject shaderObject;
		D3DReadFileToBlob(L"2DPixelShader.cso", &pBlob);
		GFX_CALL(Device->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &shaderObject.ps));

		ID3D11VertexShader* pVertexShader{0};
		D3DReadFileToBlob(L"2DVertexShader.cso", &pBlob);
		GFX_CALL(Device->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &shaderObject.vs));

		const D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
			{"Position", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"Texcoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"Color", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"Additional", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"Type", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
		GFX_CALL(Device->CreateInputLayout(layoutDesc, (uint32)std::size(layoutDesc), pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &shaderObject.il));

		Shaders[SF_2D] = shaderObject;
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

}

void Graphics::initDepthStencilStates()
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
	
	GFX_CALL(Device->CreateDepthStencilState(&dsDesc, &DepthStencilStates[DSS_Normal]));	
}

CBObject Graphics::createConstantBuffer(uint32 t_Size, void* t_InitData)
{

	CBObject cb;

	D3D11_BUFFER_DESC desc{ 0 };
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.ByteWidth = t_Size;
	desc.StructureByteStride = 0;

	// D3D11_SUBRESOURCE_DATA data{0};
	// data.pSysMem = &PixelShaderCB;

	[[maybe_unused]]HRESULT hr;
	GFX_CALL(Device->CreateBuffer(&desc, nullptr, &cb.id));

	return cb;

}

void Graphics::updateCBs()
{
	D3D11_MAPPED_SUBRESOURCE msr;

	Context->Map(VertexShaderCBId, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	memcpy(msr.pData, &VertexShaderCB, sizeof(VertexShaderCB));
	Context->Unmap(VertexShaderCBId, 0);

	Context->Map(PixelShaderCBId, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	memcpy(msr.pData, &PixelShaderCB, sizeof(PixelShaderCB));
	Context->Unmap(PixelShaderCBId, 0);

}

void Graphics::updateCBs(CBObject& t_CbObject, uint32 t_Length, void* t_Data)
{
	D3D11_MAPPED_SUBRESOURCE msr;

	[[maybe_unused]]HRESULT hr;
	GFX_CALL(Context->Map(t_CbObject.id, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr));
	memcpy(msr.pData, t_Data, t_Length);
	Context->Unmap(t_CbObject.id, 0);
}

void Graphics::updateVertexBuffer(VBObject t_Buffer, void* data, uint64 t_Length)
{
	D3D11_MAPPED_SUBRESOURCE msr;
	[[maybe_unused]]HRESULT hr;
	GFX_CALL(Context->Map(t_Buffer.id, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr));
	memcpy(msr.pData, data, t_Length);
	Context->Unmap(t_Buffer.id, 0);
}

void Graphics::updateIndexBuffer(IBObject t_Buffer, void* data, uint64 t_Length)
{
	D3D11_MAPPED_SUBRESOURCE msr;
	[[maybe_unused]]HRESULT hr;
	GFX_CALL(Context->Map(t_Buffer.id, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr));
	memcpy(msr.pData, data, t_Length);
	Context->Unmap(t_Buffer.id, 0);
}

void Graphics::bindTexture(uint32 t_Slot, TextureObject t_Texture)
{
	Context->PSSetShaderResources(t_Slot, 1, &t_Texture.srv);
}

void Graphics::setViewport(float x, float y, float width, float height)
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

void Graphics::setShaderConfiguration(ShaderConfig t_Config)
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

void Graphics::setIndexBuffer(IBObject t_buffer)
{
	Context->IASetIndexBuffer(t_buffer.id, DXGI_FORMAT_R32_UINT, 0);
}

void Graphics::setVertexBuffer(VBObject t_buffer, uint32 offset)
{
	Context->IASetVertexBuffers(0, 1, &t_buffer.id, &t_buffer.structSize, &offset);
}

void Graphics::drawIndex(TopolgyType topology, uint32 count, uint32 offset, uint32 base)
{

	// @Todo: Make this in a function of its own

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

	Context->DrawIndexed(count/factor, offset, base);
}
