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


// graphics exception checking/throwing macros (some with dxgi infos)
#define GFX_EXCEPT_NOINFO(hr) Graphics::HrException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_NOINFO(hrcall) if( FAILED( hr = (hrcall) ) ) throw Graphics::HrException( __LINE__,__FILE__,hr )

#ifndef NDEBUG
#define GFX_EXCEPT(hr) Graphics::HrException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#define GFX_THROW_INFO(hrcall) infoManager.Set(); if( FAILED( hr = (hrcall) ) ) throw GFX_EXCEPT(hr)
#define GFX_DEVICE_REMOVED_EXCEPT(hr) Graphics::DeviceRemovedException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#else
#define GFX_EXCEPT(hr) Graphics::HrException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_INFO(hrcall) GFX_THROW_NOINFO(hrcall)
#define GFX_DEVICE_REMOVED_EXCEPT(hr) Graphics::DeviceRemovedException( __LINE__,__FILE__,(hr) )
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

	class Exception : public BaseException
	{
		using BaseException::BaseException;
	};

	class HrException : public Exception
	{
	  public:
		HrException( int line,const char* file,HRESULT hr,std::vector<std::string> infoMsgs = {} ) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorString() const noexcept;
		std::string GetErrorDescription() const noexcept;
		std::string GetErrorInfo() const noexcept;
	  private:
		HRESULT hr;
		std::string info;
	};

	class DeviceRemovedException : public HrException
	{
		using HrException::HrException;
	  public:
		const char* GetType() const noexcept override;
	  private:
		std::string reason;
	};

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

	void updateCB(VSConstantBuffer buffer);
	void updateCB(PSConstantBuffer buffer);

	VBObject createVertexBuffer(size_t structSize, void* data, size_t dataSize);
	IBObject createIndexBuffer(void* data, size_t dataSize);
	template<typename Type, bool isPSBuffer>
	void createConstantBuffer(Type& buffer);

	void drawIndex(TopolgyType topology, size_t count, size_t offset = 0);

	void EndFrame();
	void ClearBuffer(float red, float green, float blue);
	void ClearZBuffer();

#ifndef NDEBUG
	DxgiInfoManager infoManager;
#endif

	ID3D11Device* m_Device{ nullptr };
	IDXGISwapChain* m_Swap{ nullptr };
	ID3D11DeviceContext* m_Context{ nullptr };
	ID3D11RenderTargetView* m_Target{ nullptr };
	ID3D11DepthStencilView* m_DepthStencilView{ nullptr };

	std::array<ShaderObject, SHADER_COUNT> m_Shaders;

	PSConstantBuffer m_PixelShaderCB;
	VSConstantBuffer m_VertexShaderCB;
	

};
