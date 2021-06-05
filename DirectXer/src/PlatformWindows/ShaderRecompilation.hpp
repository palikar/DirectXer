#pragma once

#include <Logging.hpp>
#include <Memory.hpp>
#include <Math.hpp>
#include <Utils.hpp>
#include <GraphicsContainers.hpp>
#include <GraphicsCommon.hpp>
#include <Types.hpp>
#include <Timing.hpp>

#include <dxerr.h>

#include <d3d11.h>
#include <d3d11_1.h>
#include <wrl.h>
#include <dxgi1_4.h>
#include <dxgicommon.h>
#include <d3dcompiler.h>
#include <comdef.h>

#include "PlatformWindows\IncludeWin.hpp"
#include "PlatformWindows\GraphicsD3D11.hpp"



struct ShaderReloadEntry
{
	ShaderFile ShaderProgram;
	const wchar_t* VSPath;
	const wchar_t* PSPath;
	ShaderFile PixelShadersDep[3]{SF_COUNT, SF_COUNT, SF_COUNT};
};


inline const ShaderReloadEntry shaderEntries[]
{
	{SF_DEBUG	, L"DirectXer//Shaders//VertexShader.hlsl",             L"DirectXer//Shaders//PixelShader.hlsl"},
	{SF_2D		, L"DirectXer//Shaders//2DVertexShader.hlsl",           L"DirectXer//Shaders//2DPixelShader.hlsl"},
	{SF_QUAD	, L"DirectXer//Shaders//QuadVertexShader.hlsl",         L"DirectXer//Shaders//QuadPixelShader.hlsl"},
	{SF_MTL		, L"DirectXer//Shaders//MTLVertexShader.hlsl",          L"DirectXer//Shaders//MTLPixelShader.hlsl",    {SF_MTLInst, SF_COUNT, SF_COUNT}},
	// {SF_MTLInst	, L"DirectXer//Shaders//MTLInstancedVertexShader.hlsl", nullptr,},
	{SF_PHONG	, L"DirectXer//Shaders//PhongVertexShader.hlsl",        L"DirectXer//Shaders//PhongPixelShader.hlsl"},
	{SF_TEX		, nullptr,												L"DirectXer//Shaders//TexPixelShader.hlsl"},
};

struct ShaderRecompilation
{
	static void RecompileShaders(GraphicsD3D11* Graphics)
	{
		for (size_t i = 0; i < Size(shaderEntries); ++i)
		{
			auto entry = shaderEntries[i];
			auto& shaderObject = Graphics->Shaders[entry.ShaderProgram];

			ID3DBlob* code{ nullptr };
			ID3DBlob* errors{ nullptr };
			HRESULT hr;

			auto compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;

			// if (entry.VSPath)
			// {
			// 	hr = D3DCompileFromFile(entry.VSPath, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", compileFlags, 0, &code, &errors);
			// 	if (FAILED(hr) && errors)
			// 	{
			// 		String errorMsg{(const char*)errors->GetBufferPointer() , errors->GetBufferSize() };
			// 		_bstr_t b(entry.VSPath);
			// 		const char* path = b;
			// 		DXWARNING("Compiler error for {}: \n\t {}\r", path, errorMsg);
			// 		continue;
			// 	}
			// 	else
			// 	{
			// 		shaderObject.vs->Release();
			// 		GFX_CALL(Graphics->Device->CreateVertexShader(code->GetBufferPointer(), code->GetBufferSize(), nullptr, &shaderObject.vs));
			// 	}
			// }			

			if (entry.PSPath)
			{
				hr = D3DCompileFromFile(entry.PSPath, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", compileFlags, 0, &code, &errors);
				if (FAILED(hr) && errors)
				{
					String errorMsg{ (const char*)errors->GetBufferPointer() , errors->GetBufferSize() };
					_bstr_t b(entry.PSPath);
					const char* path = b;
					DXWARNING("Compiler error for {}: \n\t {}\r", path, errorMsg);
					continue;
				}
				else
				{
					shaderObject.ps->Release();
					GFX_CALL(Graphics->Device->CreatePixelShader(code->GetBufferPointer(), code->GetBufferSize(), nullptr, &shaderObject.ps));
				}

				for (size_t i = 0; i < Size(entry.PixelShadersDep); ++i)
				{
					if (entry.PixelShadersDep[i] != SF_COUNT) Graphics->Shaders[entry.PixelShadersDep[i]].ps = shaderObject.ps;
				}

			}			

			if (code) code->Release();
			if (errors) errors->Release();
		}
	}
};
