#pragma once

#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>
#include <fstream>
#include <stb_image.h>
#include <GraphicsCommon.hpp>
#include <Types.hpp>
#include <Utils.hpp>
#include <Assets.hpp>
#include <fmt/format.h>
#include <filesystem>


static LPSTR* CommandLineToArgvA(LPSTR lpCmdLine, INT* pNumArgs)
{
	int retval;
	retval = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, lpCmdLine, -1, NULL, 0);

	LPWSTR lpWideCharStr = (LPWSTR)malloc(retval * sizeof(WCHAR));
	retval = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, lpCmdLine, -1, lpWideCharStr, retval);
	
	if (!SUCCEEDED(retval))
	{
		free(lpWideCharStr);
		return NULL;
	}
	
	int numArgs;
	LPWSTR* args;
	args = CommandLineToArgvW(lpWideCharStr, &numArgs);
	free(lpWideCharStr);

	int storage = numArgs * sizeof(LPSTR);
	for (int i = 0; i < numArgs; ++i)
	{
		BOOL lpUsedDefaultChar = FALSE;
		retval = WideCharToMultiByte(CP_ACP, 0, args[i], -1, NULL, 0, NULL, &lpUsedDefaultChar);
		if (!SUCCEEDED(retval))
		{
			LocalFree(args);
			return NULL;
		}

		storage += retval;
	}

	LPSTR* result = (LPSTR*)LocalAlloc(LMEM_FIXED, storage);
	if (result == NULL)
	{
		LocalFree(args);
		return NULL;
	}

	int bufLen = storage - numArgs * sizeof(LPSTR);
	LPSTR buffer = ((LPSTR)result) + numArgs * sizeof(LPSTR);
	for (int i = 0; i < numArgs; ++i)
	{		
		BOOL lpUsedDefaultChar = FALSE;
		retval = WideCharToMultiByte(CP_ACP, 0, args[i], -1, buffer, bufLen, NULL, &lpUsedDefaultChar);
		if (!SUCCEEDED(retval))
		{
			LocalFree(result);
			LocalFree(args);
			return NULL;
		}

		result[i] = buffer;
		buffer += retval;
		bufLen -= retval;
	}

	LocalFree(args);

	*pNumArgs = numArgs;
	return result;

}

struct AtlasViewer
{
	struct CommandLineArguments
	{
		std::string Root{"resources"};
		std::string Input{"input.dxa"};
	};

};

struct Context
{
	HWND hWnd;
	AtlasViewer::CommandLineArguments Args;
	bool FullscreenMode;
	UINT WindowStyle;
	RECT WindowRect;
	Graphics Graphics;
	TextureId DummyTex;
	uint16 currentTex{0};
	std::vector<TextureId> Texs;
};

static void HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, Context& context);

static LRESULT CALLBACK HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_CREATE)
	{
		auto context = (Context*)((LPCREATESTRUCTA)lParam)->lpCreateParams;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)context);

		context->Graphics.InitSwapChain(hWnd, 1080, 720);
		context->Graphics.InitBackBuffer();
		context->Graphics.InitZBuffer(1080, 720);
		context->Graphics.InitResources();
		context->Graphics.InitRasterizationsStates();
		context->Graphics.InitSamplers();
		context->Graphics.InitBlending();
		context->Graphics.InitDepthStencilStates();
	}
	else
	{
		auto context = (Context*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		HandleMessage(hWnd, msg, wParam, lParam, *context);
	}
	
	return DefWindowProc(hWnd, msg, wParam, lParam);

}
