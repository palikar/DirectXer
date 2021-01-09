#include "Graphics.hpp"


#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

void Graphics::initSwapChain(HWND hWnd)
{
	
	DXGI_SWAP_CHAIN_DESC sd{ 0 };
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
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


	HRESULT hr;
    GFX_CALL(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
	D3D11_CREATE_DEVICE_DEBUG, nullptr, 0, D3D11_SDK_VERSION, &sd, &m_Swap, &m_Device, nullptr, &m_Context));
	
}

void Graphics::initBackBuffer()
{

	ID3D11Resource* pBackBuffer{ nullptr };

	HRESULT hr;
    GFX_CALL(m_Swap->GetBuffer(0, __uuidof(ID3D11Resource), (void**)(&pBackBuffer)));
	GFX_CALL(m_Device->CreateRenderTargetView(pBackBuffer, nullptr, &m_Target));
	pBackBuffer->Release();
}

void Graphics::initZBuffer()
{
	HRESULT hr;
	
	D3D11_DEPTH_STENCIL_DESC dsDesc{0};
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
    
    ID3D11DepthStencilState *pDSState;
	GFX_CALL(m_Device->CreateDepthStencilState(&dsDesc, &pDSState));
	m_Context->OMSetDepthStencilState(pDSState, 1);
	

	ID3D11Texture2D *pDepthStencil;
	D3D11_TEXTURE2D_DESC descDepth{0};
	descDepth.Width = 800u;
	descDepth.Height = 600u;
	descDepth.MipLevels = 1u;
	descDepth.ArraySize = 1u;
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count = 1u;
	descDepth.SampleDesc.Quality = 0u;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	GFX_CALL( m_Device->CreateTexture2D( &descDepth,nullptr,&pDepthStencil ) );

    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV{0};
	descDSV.Format = DXGI_FORMAT_D32_FLOAT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0u;
	GFX_CALL(m_Device->CreateDepthStencilView(pDepthStencil, &descDSV, &m_DepthStencilView));
 
    m_Context->OMSetRenderTargets(1, &m_Target, m_DepthStencilView);
	assert(m_DepthStencilView);

}

void Graphics::Destroy()
{
	
	m_Target->Release();
    m_Context->Release();
    m_Swap->Release();
    m_Device->Release();
    m_DepthStencilView->Release();
}

void Graphics::EndFrame()
{
    HRESULT hr;
	if( FAILED( hr = m_Swap->Present( 1u,0u ) ) )
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

	m_Context->ClearRenderTargetView(m_Target, color);
}

void Graphics::ClearZBuffer()
{
	m_Context->ClearDepthStencilView(m_DepthStencilView, D3D11_CLEAR_DEPTH | D3D10_CLEAR_STENCIL, 1.0f, 0u);
}

void Graphics::setRasterizationState()
{

	ID3D11RasterizerState* rastState;
	D3D11_RASTERIZER_DESC rastDesc{0};
	rastDesc.CullMode = D3D11_CULL_NONE;
	rastDesc.FillMode = D3D11_FILL_WIREFRAME;
	rastDesc.FrontCounterClockwise = false;
	rastDesc.ScissorEnable  = false;

	HRESULT hr;
	GFX_CALL(m_Device->CreateRasterizerState(&rastDesc, &rastState));
	m_Context->RSSetState(rastState);

}

VBObject Graphics::createVertexBuffer(uint32 structSize, void* data, uint32 dataSize)
{
	ID3D11Buffer* pVertexBuffer;
	D3D11_BUFFER_DESC vertexBufferDesc{ 0 };
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.ByteWidth = dataSize;
	vertexBufferDesc.StructureByteStride = structSize;

	D3D11_SUBRESOURCE_DATA bufferData;
	bufferData.pSysMem = data;

	HRESULT hr;
	GFX_CALL(m_Device->CreateBuffer(&vertexBufferDesc, &bufferData, &pVertexBuffer));

	return {structSize, pVertexBuffer};
}

IBObject Graphics::createIndexBuffer(void* data, uint32 dataSize)
{
	ID3D11Buffer* pIndexBuffer;
	D3D11_BUFFER_DESC indexBufferDesc{ 0 };
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.ByteWidth = dataSize;
	indexBufferDesc.StructureByteStride = sizeof(uint32);

	D3D11_SUBRESOURCE_DATA indexBufferData{0};
	indexBufferData.pSysMem = data;

	HRESULT hr;
	GFX_CALL(m_Device->CreateBuffer(&indexBufferDesc, &indexBufferData, &pIndexBuffer));

	return {pIndexBuffer};
}

void Graphics::initResources()
{

	ID3DBlob* pBlob;
	ID3D11PixelShader* pPixelShader{0};
	D3DReadFileToBlob(L"PixelShader.cso", &pBlob);

	HRESULT hr;
	GFX_CALL(m_Device->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPixelShader));

	ID3D11VertexShader* pVertexShader{0};
	D3DReadFileToBlob(L"VertexShader.cso", &pBlob);
	GFX_CALL(m_Device->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pVertexShader));

	ID3D11InputLayout* pInputLayout;
    const D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        {"Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	
	GFX_CALL(m_Device->CreateInputLayout(layoutDesc, (uint32)std::size(layoutDesc), pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &pInputLayout));
	
	m_Shaders[0] = ShaderObject{pInputLayout, pVertexShader, pPixelShader};


	D3D11_BUFFER_DESC vertexShaderCBDesc{ 0 };
    vertexShaderCBDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexShaderCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	vertexShaderCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexShaderCBDesc.MiscFlags = 0;
	vertexShaderCBDesc.ByteWidth = sizeof(VSConstantBuffer);
	vertexShaderCBDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA vertexShaderCBData{0};
	vertexShaderCBData.pSysMem = &m_VertexShaderCB;
    GFX_CALL(m_Device->CreateBuffer(&vertexShaderCBDesc, &vertexShaderCBData, &VSConstantBuffer::id));
	m_Context->VSSetConstantBuffers(0, 1, &VSConstantBuffer::id);
	

    D3D11_BUFFER_DESC pixelShaderCBDesc{ 0 };
    pixelShaderCBDesc.Usage = D3D11_USAGE_DYNAMIC;
	pixelShaderCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	pixelShaderCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pixelShaderCBDesc.MiscFlags = 0;
	pixelShaderCBDesc.ByteWidth = sizeof(PSConstantBuffer);
	pixelShaderCBDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA pixelShaderCBData{0};
    pixelShaderCBData.pSysMem = &m_PixelShaderCB;
    GFX_CALL(m_Device->CreateBuffer(&pixelShaderCBDesc, &pixelShaderCBData, &PSConstantBuffer::id));
	m_Context->PSSetConstantBuffers(0, 1, &PSConstantBuffer::id);	
	
}


void Graphics::updateCBs()
{
	D3D11_MAPPED_SUBRESOURCE msr;

	m_Context->Map(VSConstantBuffer::id, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	memcpy(msr.pData, &m_VertexShaderCB, sizeof(m_VertexShaderCB));
	m_Context->Unmap(VSConstantBuffer::id, 0);

	m_Context->Map(PSConstantBuffer::id, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	memcpy(msr.pData, &m_PixelShaderCB, sizeof(m_PixelShaderCB));
	m_Context->Unmap(PSConstantBuffer::id, 0);	
	
}

template<typename Type, bool isPSBuffer>
void Graphics::createConstantBuffer(Type& buffer)
{

	ID3D11Buffer* constantBuffer{0};
    D3D11_BUFFER_DESC constantBufferDesc{ 0 };
    constantBuffer.Usage = D3D11_USAGE_DYNAMIC;
	constantBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBuffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantBuffer.MiscFlags = 0;
	constantBuffer.ByteWidth = sizeof(Type);
	constantBuffer.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA constantBufferData{0};
    constantBuffer.pSysMem = &buffer;

	HRESULT hr;
	GFX_CALL(m_Device->CreateBuffer(&constantBufferDesc, &constantBufferData, &Type::id));
	
	if constexpr (isPSBuffer) {
		m_Context->PSSetConstantBuffers(0, 1, &Type::id);
	}else {
		m_Context->VSSetConstantBuffers(0, 1, &Type::id);
	}
	
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

	m_Context->RSSetViewports(1, &vp);

	// D3D11_RECT sissorRect;
	// sissorRect.bottom = 0;
	// sissorRect.left= 0;
	// sissorRect.top= height;
	// sissorRect.right= width;
	// m_Context->RSSetScissorRects(1, &sissorRect);
	
}

void Graphics::setShaders(ShaderType t_Shader)
{

	auto& shaderObject = m_Shaders[t_Shader];

	m_Context->PSSetShader(shaderObject.ps, nullptr, 0);
	m_Context->VSSetShader(shaderObject.vs, nullptr, 0);
	m_Context->IASetInputLayout(shaderObject.il);

}

void Graphics::setIndexBuffer(IBObject t_buffer)
{
	m_Context->IASetIndexBuffer(t_buffer.id, DXGI_FORMAT_R32_UINT, 0);
}

void Graphics::setVertexBuffer(VBObject t_buffer, uint32 offset)
{
	m_Context->IASetVertexBuffers(0, 1, &t_buffer.id, &t_buffer.structSize, &offset);
}

void Graphics::drawIndex(TopolgyType topology, uint32 count, uint32 offset, uint32 base)
{
	switch (topology) {
	  case TT_TRIANGLES: 
		  m_Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		  break;
	}
	
	m_Context->DrawIndexed(count, offset, base);
}


