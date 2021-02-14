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
#include <time.h>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#include <fmt/format.h>

static void SetupConsole()
{

	AllocConsole();

	FILE* fDummy;
	freopen_s(&fDummy, "CONIN$", "r", stdin);
	freopen_s(&fDummy, "CONOUT$", "w", stderr);
	freopen_s(&fDummy, "CONOUT$", "w", stdout);

}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT CALLBACK HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_CREATE:
	{
		auto app = (App*)((LPCREATESTRUCTA)lParam)->lpCreateParams;

		// @Todo: Move this in init code somewhere
		app->Init(hWnd);
		ImGui_ImplWin32_Init(hWnd);
		ImGui_ImplDX11_Init(app->Graphics.Device, app->Graphics.Context);
		
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

static LPSTR* CommandLineToArgvA(LPSTR lpCmdLine, INT* pNumArgs)
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

static void ParseCommandLineArguments(CommandLineSettings& t_Settings, char** argv, int argc)
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

static double clockToMilliseconds(clock_t ticks){
    return (ticks/(double)CLOCKS_PER_SEC)*1000.0;
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

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();


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
	
	clock_t deltaTime = 0;
	clock_t dt = 0;
	unsigned int frames = 0;
	double  frameRate = 30;
	double  averageFrameTimeMilliseconds = 33.333;


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

		clock_t beginFrame = clock();
		
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();


		application.Spin((float)clockToMilliseconds(dt) / 1000.0f);

		
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		application.Graphics.EndFrame();

		clock_t endFrame = clock();

		dt = endFrame - beginFrame;
		deltaTime += endFrame - beginFrame;
		frames++;

		if( clockToMilliseconds(deltaTime)>1000.0)
		{
			frameRate = (double)frames*0.5 +  frameRate*0.5; //more stable
			deltaTime -= CLOCKS_PER_SEC;
			averageFrameTimeMilliseconds  = 1000.0/(frameRate ==0 ? 0.001 : frameRate);

			DXLOG("[SYS] Frame time: {:.2} ms; FPS: {} ", averageFrameTimeMilliseconds, frames);
			
			frames = 0;
		}

	}

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

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
// @Done: Shader types setup
// -> Shader files(switching the actual shaders), Shader types(value in a CB)
// -> Debug Shader -- can draw with color or texture mapped (to test materials and stuff)
// @Done: Input layout tpyes setup
// -- debug layout (pos, color, uv, normals, tangents, bitangents)
// -- standard layout (pos, uv, normals, tangents, bitangents)
// @Done: Constant buffers setup
// @Todo: Depth Stencil State Setup
// @Todo: Scissor test support
// @Todo: Render target support

// @Done: Lines "geometry" + ability to use lines primiteves
// @Done: Cylinsder geometry
// @Done: Cone geometry
// @Done: Torus geometry

// @Rendering
// @Done: Rendering sky box
// @Todo: Rendering groups
// @Todo: Rendering fog
// @Done: Material setup
// @Done: Phong material
// @Done: Sollid color material
// @Todo: PBR material
// @Done: Point light
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
// @Done: Point light helper
// @Done: FPS Camara controller
// @Done: Adjust all of the geometry generators
// @Done: Add the normals and uvs to the color vertex
// @Todo: Spot light helper
// @Todo: Drop down console

// @Architecture
// @Done: Resizing
// @Done: FPS independence
// @Todo: Some job\task system for basic multithreading support

// @Notes
// @Note: In a scene, some things are dynamic, and some things are static


