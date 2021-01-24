#include "IncludeWin.hpp"
#include "Input.hpp"
#include "Glm.hpp"
#include "App.hpp"
#include "Types.hpp"
#include "Logging.hpp"

#include <fcntl.h>
#include <stdio.h>
#include <io.h>
#include <iostream>
#include <Stringapiset.h>
#include <shellapi.h>

#include <fmt/format.h>

void SetupConsole()
{

	AllocConsole();

	FILE* fDummy;
	freopen_s(&fDummy, "CONIN$", "r", stdin);
	freopen_s(&fDummy, "CONOUT$", "w", stderr);
	freopen_s(&fDummy, "CONOUT$", "w", stdout);

}

LRESULT CALLBACK HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch (msg)
	{
	case WM_CREATE:
	{
		auto app = (App*)((LPCREATESTRUCTA)lParam)->lpCreateParams;
		app->Init(hWnd);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)app);
		break;
	}

	case WM_CLOSE:
	{
		PostQuitMessage(0);
		return 0;
	}


	case WM_SIZE:
	{
		auto app = (App*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		
		// @Todo: Handle minimize here
		uint32 width = LOWORD(lParam);
		uint32 height = HIWORD(lParam);
		app->Width = (float32)width;
		app->Height = (float32)height;
		app->Resize();
		
		DXLOG("[EVENT] Resize");
		return 0;
	}


	case WM_KILLFOCUS:
	{
		gInput.Reset();
	}

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	{
		if (!(lParam & 0x40000000))
		{
			gInput.UpdateKeyboardButtonPressed(static_cast<uint16_t>(wParam));
		}
		break;
	}

	case WM_KEYUP:
	case WM_SYSKEYUP:
	{
		
		gInput.UpdateKeyboardButtonReleased(static_cast<uint16_t>(wParam));
		break;
	}

	case WM_RBUTTONUP:
	{
		gInput.UpdateMouseButtonReleased(1);
		break;
	}
	case WM_LBUTTONUP:
	{
		gInput.UpdateMouseButtonReleased(0);
		break;
	}
	case WM_MBUTTONUP:
	{
		gInput.UpdateMouseButtonReleased(2);
		break;
	}

	case WM_LBUTTONDOWN:
	{
		gInput.UpdateMouseButtonPressed(0);
		break;
	}
	case WM_RBUTTONDOWN:
	{
		gInput.UpdateMouseButtonPressed(1);
		break;
	}
	case WM_MBUTTONDOWN:
	{
		gInput.UpdateMouseButtonPressed(2);
		break;
	}

	case WM_MOUSEMOVE:
	{
		const POINTS pt = MAKEPOINTS(lParam);
		gInput.UpdateMousePos({ pt.x, pt.y });
		break;
	}

	case WM_MOUSEWHEEL:
	{
		const POINTS pt = MAKEPOINTS(lParam);
		const int delta = GET_WHEEL_DELTA_WPARAM(wParam);
		gInput.UpdateMouseScroll((float)delta);
		break;
	}


	}

	return DefWindowProc(hWnd, msg, wParam, lParam);

}

LPSTR* CommandLineToArgvA(LPSTR lpCmdLine, INT* pNumArgs)
{
	int retval;
	retval = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, lpCmdLine, -1, NULL, 0);
	if (!SUCCEEDED(retval))
		return NULL;

	LPWSTR lpWideCharStr = (LPWSTR)malloc(retval * sizeof(WCHAR));
	if (lpWideCharStr == NULL)
		return NULL;

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
	if (args == NULL)
		return NULL;

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
		assert(bufLen > 0);
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

void ParseCommandLineArguments(CommandLineSettings& t_Settings, char** argv, int argc)
{

	for (size_t i = 1; i < argc; ++i)
	{
		if (strcmp(argv[i], "--resources") == 0)
		{
			DXLOG("[INIT] Argument: {} -> {}", argv[i], argv[i + 1]);
			t_Settings.ResourcesPath = argv[i + 1];
			++i;
		}

		
	}


}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdSHow)
{

	SetupConsole();
	gInput.Init();
	gDxgiManager.Init();

	App application;

	char** argv;
	int argc;
	argv = CommandLineToArgvA(GetCommandLine(), &argc);
	ParseCommandLineArguments(application.Arguments, argv, argc);
	
	WNDCLASSEX windowClass{ 0 };
	windowClass.cbSize = sizeof(windowClass);
	windowClass.style = CS_OWNDC;
	windowClass.lpfnWndProc = HandleMsg;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = hInstance;
	windowClass.hIcon = nullptr;
	windowClass.hCursor = nullptr;
	windowClass.hbrBackground = nullptr;
	windowClass.lpszMenuName = nullptr;
	windowClass.lpszClassName = "DirectXer Window";
	windowClass.hIconSm = nullptr;
	RegisterClassEx(&windowClass);

	
	uint32 INITIAL_WIDTH = 800;
	uint32 INITIAL_HEIGHT = 600;

	// @Note: Implement fullscreen init here
	DWORD windowStyle = WS_THICKFRAME | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;
	RECT windowRect;
	windowRect.left = 100;
	windowRect.right = INITIAL_WIDTH + windowRect.left;
	windowRect.top = 100;
	windowRect.bottom = INITIAL_HEIGHT + windowRect.top;

	if (AdjustWindowRect(&windowRect, windowStyle, FALSE) == 0)
	{
		return 0;
	}


	application.Width = (float32)INITIAL_WIDTH;
	application.Height = (float32)INITIAL_HEIGHT;


	auto windowHanlde = CreateWindow(
		"DirectXer Window", "DirectXer", windowStyle,
		CW_USEDEFAULT, CW_USEDEFAULT, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
		nullptr, nullptr, hInstance, &application);


	if (windowHanlde == nullptr)
	{
		return 0;
	}


	ShowWindow(windowHanlde, SW_SHOWDEFAULT);

	//SetWindowText(hWnd, title.c_str())

	MSG msg;
	while (true)
	{
		gInput.Update();

		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				return (int)msg.wParam;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		application.Spin();

	}


	gDxgiManager.Destroy();
	application.Destroy();
	UnregisterClass("DirectXer Window", hInstance);
	FreeConsole();
	DestroyWindow(windowHanlde);

	return 0;
}

// @Memory magament
// @Todo: Buffering the logging output
// @Todo: Allcating some amount of memory upfront
// @Todo: Pool temporary allocation strategy
// @Todo: Temporary Vector and Map
// @Todo: Have a separate palce for "debug" memory

// @Done: Ability to transform vertices on the CPU
// @Done: Proper Vertex struct format
// @Done: Transformation per vertex type
// @Done: Robing hood map from the github repo
// @Done: Basic setup for command line arguments passig

// @Graphics
// @Done: Loading textures from file
// @Done: Creating textures in the graphics
// @Done: Rasterization state types setup
// @Todo: Shader types setup
// -> Shader files(switching the actual shaders), Shader types(value in a CB)
// -> Debug Shader -- can draw with color or texture mapped (to test materials and stuff)
// @Todo: Scissor test support
// @Todo: Input layout tpyes setup
// -- debug layout (pos, color, uv, normals, tangents, bitangents)
// -- standard layout (pos, uv, normals, tangents, bitangents)


// @Done: Lines "geometry" + ability to use lines primiteves
// @Done: Cylinsder geometry
// @Todo: Cone geometry
// @Todo: Torus geometry

// @Rendering
// @Todo: Rendering sky box
// @Todo: Rendering groups
// @Todo: Rendering fog
// @Todo: Phong material
// @Todo: PBR material
// @Todo: Sollid color material
// @Todo: Point light
// @Todo: Text rendering

// @Data management
// @Done: Container for geometry info!
// @Done: Color vertex type -- hence solid color material (ish)
// @Todo: Loading obj files
// @Todo: Mouse picking
// @Done: Texture catalog but a good one, not holding textures in memory all the time
// @Todo: Tags for the textures -- for certain level, for certain material instance
//     -- load all textures in a level
//     -- get textures for some material instance

// @Debug
// @Done: Axis helper
// @Done: Grid Helper
// @Todo: Spot light helper
// @Todo: Point light helper
// @Todo: Drop down console
// @Done: FPS Camara controller
// @Todo: Adjust all of the geometry generators
// @Todo: Add the normals and uvs to the color vertex

// @Architecture
// @Done: Resizing
// @Todo: Some job\task system for basic multithreading support

// @Notes
// @Note: In a scene, some things are dynamic, and some things are static


