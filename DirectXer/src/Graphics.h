#pragma once

#include "IncludeWin.h"
#include "BaseException.h"
#include "DxgiInfoManager.h"

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

#include "GraphicsCommon.h"


#ifndef NDEBUG
#define GFX_CALL(hrcall) if( FAILED( hr = (hrcall) ) ) assert(false && #hrcall);
#else
#define GFX_CALL(hrcall) (hrcall)
#endif

namespace dx = DirectX;

struct VBObject
{
	size_t structSize;
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

	enum TopolgyType
	{
		TT_TRIANGLES = 0
	};


	~Graphics();

	void initSwapChain(HWND hWnd);
	void initBackBuffer();
	void initZBuffer();
	void initResources();

	void setRasterizationState();
	void setShaders(ShaderType t_Shader);
	void setVertexBuffer(VBObject t_buffer, size_t offset = 0);
	void setIndexBuffer(IBObject t_buffer);
	void setViewport(float x, float y, float width, float height);

	void updateCBs();

	VBObject createVertexBuffer(size_t structSize, void* data, size_t dataSize);
	IBObject createIndexBuffer(void* data, size_t dataSize);
	template<typename Type, bool isPSBuffer>
	void createConstantBuffer(Type& buffer);

	void drawIndex(TopolgyType topology, size_t count, size_t offset = 0,  size_t base = 0);

	void EndFrame();
	void ClearBuffer(float red, float green, float blue);
	void ClearZBuffer();

// #ifndef NDEBUG
// 	DxgiInfoManager infoManager;
// #endif

	ID3D11Device* m_Device{ nullptr };
	IDXGISwapChain* m_Swap{ nullptr };
	ID3D11DeviceContext* m_Context{ nullptr };
	ID3D11RenderTargetView* m_Target{ nullptr };
	ID3D11DepthStencilView* m_DepthStencilView{ nullptr };

	std::array<ShaderObject, SHADER_COUNT> m_Shaders;

	PSConstantBuffer m_PixelShaderCB;
	VSConstantBuffer m_VertexShaderCB;
	

};

template<typename VertexType>
VBObject vertexBufferFactory(Graphics& graphics, std::vector<float> t_VertexList)
{
	return graphics.createVertexBuffer(sizeof(VertexType), t_VertexList.data(), sizeof(VertexType)*t_VertexList.size());
}

template<typename IndexType = size_t>
IBObject indexBufferFactory(Graphics& graphics, std::vector<IndexType> t_IndexList)
{
	return graphics.createIndexBuffer(t_IndexList.data(), sizeof(IndexType)*t_IndexList.size());
}
