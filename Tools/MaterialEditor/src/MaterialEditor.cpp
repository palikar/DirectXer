#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>
#include <fstream>
#include <cmath>

#include <fmt/format.h>
#include <filesystem>
#include <stb_image.h>
#include <stb_image_resize.h>
#include <stb_rect_pack.h>

#include "MaterialEditor.hpp"
#include "Editor.hpp"

#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

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

static void ToggleFullscreen(Context& context)
{

	if(context.FullscreenMode)
	{
		SetWindowLong(context.hWnd, GWL_STYLE, context.WindowStyle);
		SetWindowPos(context.hWnd, HWND_NOTOPMOST, context.WindowRect.left, context.WindowRect.top,
					 context.WindowRect.right - context.WindowRect.left,
					 context.WindowRect.bottom - context.WindowRect.top,
					 SWP_FRAMECHANGED | SWP_NOACTIVATE);
        ShowWindow(context.hWnd, SW_NORMAL);
	}
	else
	{
		GetWindowRect(context.hWnd, &context.WindowRect);
		SetWindowLong(context.hWnd, GWL_STYLE, context.WindowStyle & ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME));

		RECT fullscreenWindowRect;
		IDXGIOutput* Output;
		context.Graphics.Swap->GetContainingOutput(&Output);
		DXGI_OUTPUT_DESC Desc;
		Output->GetDesc(&Desc);
		fullscreenWindowRect = Desc.DesktopCoordinates;
		Output->Release();

		SetWindowPos(context.hWnd, HWND_TOPMOST, fullscreenWindowRect.left, fullscreenWindowRect.top,
					 fullscreenWindowRect.right, fullscreenWindowRect.bottom,
					 SWP_FRAMECHANGED | SWP_NOACTIVATE);
        ShowWindow(context.hWnd, SW_MAXIMIZE);		
	}
	
	context.FullscreenMode = !context.FullscreenMode;
}	


static void HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, Context& context)
{
	switch (msg)
	{
	  case WM_CLOSE:
	  {
		  PostQuitMessage(0);
	  }

	  case WM_SIZE:
	  {
		  if(wParam == SIZE_MINIMIZED)
		  {
			  DXDEBUG("[Event] Minimizing");
			  break;
		  }

		  uint32 width = LOWORD(lParam);
		  uint32 height = HIWORD(lParam);
		  if (width == 0 || height == 0) break;

		  context.Graphics.ResizeBackBuffer((float)width, (float)height);
		  context.Graphics.DestroyZBuffer();
		  context.Graphics.InitZBuffer((float)width, (float)height);
		  context.Graphics.SetViewport(0, 0, (float)width, (float)height);

		  context.Height = (float)height;
		  context.Width = (float)width;

		  break;
	  }

	  case WM_KILLFOCUS:
	  {
		  Input::gInput.Reset();
	  }

	  case WM_KEYDOWN:
	  case WM_SYSKEYDOWN:
	  {
		  if (!(lParam & 0x40000000))
		  {
			  Input::gInput.UpdateKeyboardButtonPressed(static_cast<uint16_t>(wParam));
		  }
		  break;
	  }

	  case WM_KEYUP:
	  case WM_SYSKEYUP:
	  {
		
		  Input::gInput.UpdateKeyboardButtonReleased(static_cast<uint16_t>(wParam));
		  break;
	  }

	  case WM_RBUTTONUP:
	  {
		  Input::gInput.UpdateMouseButtonReleased(1);
		  break;
	  }
	  case WM_LBUTTONUP:
	  {
		  Input::gInput.UpdateMouseButtonReleased(0);
		  break;
	  }
	  case WM_MBUTTONUP:
	  {
		  Input::gInput.UpdateMouseButtonReleased(2);
		  break;
	  }

	  case WM_LBUTTONDOWN:
	  {
		  Input::gInput.UpdateMouseButtonPressed(0);
		  break;
	  }
	  case WM_RBUTTONDOWN:
	  {
		  Input::gInput.UpdateMouseButtonPressed(1);
		  break;
	  }
	  case WM_MBUTTONDOWN:
	  {
		  Input::gInput.UpdateMouseButtonPressed(2);
		  break;
	  }

	  case WM_MOUSEMOVE:
	  {
		  const POINTS pt = MAKEPOINTS(lParam);
		  Input::gInput.UpdateMousePos({ pt.x, pt.y });
		  break;
	  }

	  case WM_MOUSEWHEEL:
	  {
		  const POINTS pt = MAKEPOINTS(lParam);
		  const int delta = GET_WHEEL_DELTA_WPARAM(wParam);
		  Input::gInput.UpdateMouseScroll((float)clamp(delta, -120, 120));
		  break;
	  }

	}
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT CALLBACK HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return false;
	
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

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::StyleColorsDark();

		ImGui_ImplWin32_Init(hWnd);
		ImGui_ImplDX11_Init(context->Graphics.Device, context->Graphics.Context);
	}
	else
	{
		auto context = (Context*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		HandleMessage(hWnd, msg, wParam, lParam, *context);
	}
	
	return DefWindowProc(hWnd, msg, wParam, lParam);

}

static void ParseCommandLineArguments(int argc, char *argv[], MaterialEditor::CommandLineArguments& arguments)
{
	for (size_t i = 0; i < argc; ++i)
	{
		std::string current{argv[i]};
		if(current == "-r") {
			arguments.Root = argv[++i];
		} else if (current == "-i") {
			arguments.Input = argv[++i];
		}
	}
}

static double clockToMilliseconds(clock_t ticks)
{
    return (ticks/(double)CLOCKS_PER_SEC)*1000.0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdSHow)
{
	SetupConsole();
	gDxgiManager.Init();
	Memory::InitMemoryState();
	Input::Init();
	PlatformLayer::Init();

	int argc;
	char** argv;
	argv = CommandLineToArgvA(GetCommandLine(), &argc);

	Context context;
	ParseCommandLineArguments(argc, argv, context.Args);

	WNDCLASSEX windowClass{ 0 };
	windowClass.cbSize = sizeof(windowClass);
	windowClass.style = CS_OWNDC;
	windowClass.lpfnWndProc = HandleMsg;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GetModuleHandleA(NULL);
	windowClass.hIcon = nullptr;
	windowClass.hCursor = nullptr;
	windowClass.hbrBackground = nullptr;
	windowClass.lpszMenuName = nullptr;
	windowClass.lpszClassName = "MaterialEdit";
	windowClass.hIconSm = nullptr;
	RegisterClassEx(&windowClass);

	context.WindowStyle = WS_THICKFRAME | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;

	context.WindowRect.left = 100;
	context.WindowRect.right = 1080 + context.WindowRect.left;
	context.WindowRect.top = 100;
	context.WindowRect.bottom = 720 + context.WindowRect.top;

	context.hWnd = CreateWindow("MaterialEdit", "Material Editor", context.WindowStyle,
							 CW_USEDEFAULT, CW_USEDEFAULT, context.WindowRect.right - context.WindowRect.left,
							 context.WindowRect.bottom - context.WindowRect.top,
							 nullptr, nullptr, GetModuleHandleA(NULL), &context);

	DragAcceptFiles(context.hWnd, TRUE);
	ShowWindow(context.hWnd, SW_SHOW);
	
	Init(context);

	ToggleFullscreen(context);

	clock_t dt = 0;
	
	while (true)
	{
		Input::gInput.Update();
		
		MSG msg;
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				return (int)msg.wParam;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (Input::gInput.IsKeyReleased(KeyCode::F11))
		{
			ToggleFullscreen(context);
		}
		
		clock_t beginFrame = clock();
		
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("DX Materials");
		const float delta = (float)clockToMilliseconds(dt) / 1000.0f;
		Update(context, delta);

		ImGui::End();

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		context.Graphics.EndFrame();

		clock_t endFrame = clock();

		dt = endFrame - beginFrame;
	}

	return 0;
}
