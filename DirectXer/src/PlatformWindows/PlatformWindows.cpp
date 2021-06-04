#include <App.hpp>
#include <Input.hpp>
#include <Glm.hpp>
#include <Logging.hpp>
#include <Timing.hpp>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <fmt/format.h>
#include <Xinput.h>

#include <Shlwapi.h>
#include <time.h>

#include "PlatformWindows.hpp"

void WindowsPlatformLayer::Init()
{
	StdOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	ErrOutHandle = GetStdHandle(STD_ERROR_HANDLE);
}

void WindowsPlatformLayer::SetOuputColor(ConsoleForeground color)
{
	SetConsoleTextAttribute(StdOutHandle, (WORD)color);
}
		
void WindowsPlatformLayer::WriteStdOut(const char* data, size_t len)
{
	WriteFile(StdOutHandle, data, (DWORD)len, NULL, NULL);
}

void WindowsPlatformLayer::WriteErrOut(const char* data, size_t len)
{
	WriteFile(ErrOutHandle, data, (DWORD)len, NULL, NULL);
}
	
void* WindowsPlatformLayer::Allocate(size_t t_Size)
{
	return VirtualAlloc(NULL, t_Size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

uint64 WindowsPlatformLayer::Clock()
{
	ULONGLONG lpInterruptTimePrecise;
	QueryInterruptTimePrecise(&lpInterruptTimePrecise);
	return lpInterruptTimePrecise;
}

WindowsPlatformLayer::FileHandle WindowsPlatformLayer::OpenFileForReading(const char* t_Path)
{
	FileHandle handle = CreateFile(t_Path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE)
	{
		DXERROR("Can't open file: {} ", t_Path);
	}
	return handle;
}

WindowsPlatformLayer::FileHandle WindowsPlatformLayer::OpenFileForWriting(const char* t_Path)
{
	FileHandle handle = CreateFile(t_Path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE)
	{
		DXERROR("Can't open file: {} ", t_Path);
	}
	return handle;
}

size_t WindowsPlatformLayer::FileSize(FileHandle handle)
{
	DWORD fileSize = GetFileSize(handle, NULL);
	return fileSize;
}

bool WindowsPlatformLayer::IsValidPath(const char* path)
{
	return PathFileExists(path);
}

void WindowsPlatformLayer::ReadFileIntoArena(FileHandle handle, size_t size, MemoryArena& t_Arena)
{
	DWORD readBytes;
	ReadFile(handle, t_Arena.Memory + t_Arena.Size, (DWORD)size, &readBytes, NULL);
	Assert(readBytes <= t_Arena.MaxSize - t_Arena.Size, "Can't read the whole file into the given arena.");
	t_Arena.Size += readBytes;
}

void WindowsPlatformLayer::WriteArenaIntoFile(FileHandle handle, MemoryArena& t_Arena)
{
	DWORD readBytes;
	WriteFile(handle, t_Arena.Memory, (DWORD)t_Arena.Size, &readBytes, NULL);

	// @Note: maybe save this for later
	FlushFileBuffers(handle);
}

void WindowsPlatformLayer::CloseFile(FileHandle handle)
{
	// @Todo: Probably queue this in some vector and close them in one go
	// at some point
	 CloseHandle(handle);
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT CALLBACK HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	if (msg == WM_CREATE)
	{
		auto wind = (WindowsWindow*)((LPCREATESTRUCTA)lParam)->lpCreateParams;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)wind);
		wind->InitAfterCreate(hWnd);
	}
	else
	{
		auto wind = (WindowsWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		wind->HandleMessage(hWnd, msg, wParam, lParam);
	}
	
	return DefWindowProc(hWnd, msg, wParam, lParam);

}

static double clockToMilliseconds(clock_t ticks)
{
    return (ticks/(double)CLOCKS_PER_SEC)*1000.0;
}

static RENDERDOC_API_1_3_0* InitRenderDoc()
{
	RENDERDOC_API_1_3_0 *rdoc_api = NULL;
	
	HMODULE mod = GetModuleHandleA(TEXT("renderdoc.dll"));
	if(!mod) return nullptr;
	
	pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
    int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void **)&rdoc_api);
    assert(ret == 1);

	return rdoc_api;
}

void WindowsWindow::Init(WindowsSettings t_Settings)
{
	Settings = t_Settings;
	FullscreenMode = false;
	Minimized = false;

	Rdoc =	InitRenderDoc();
	
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
	windowClass.lpszClassName = t_Settings.ClassName;
	windowClass.hIconSm = nullptr;
	RegisterClassEx(&windowClass);

	WindowStyle = WS_THICKFRAME | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;

	WindowRect.left = 100;
	WindowRect.right = Settings.InitialWidth + WindowRect.left;
	WindowRect.top = 100;
	WindowRect.bottom = Settings.InitialHeight + WindowRect.top;

	Assert(AdjustWindowRect(&WindowRect, WindowStyle, FALSE) != 0, "Can't adjust the window's rectangle");
		
	Application->Width = (float32)Settings.InitialWidth;
	Application->Height = (float32)Settings.InitialHeight;

	hWnd = CreateWindow(t_Settings.ClassName, t_Settings.WindowTitle, WindowStyle,
						CW_USEDEFAULT, CW_USEDEFAULT, WindowRect.right - WindowRect.left,
						WindowRect.bottom - WindowRect.top,
						nullptr, nullptr, GetModuleHandleA(NULL), this);
	Assert(hWnd, "Can't open window");

	ShowWindow(hWnd, SW_SHOW);

	Application->PostInit();
}

void WindowsWindow::InitAfterCreate(HWND t_hWnd)
{
	if (Rdoc) Rdoc->StartFrameCapture(NULL, NULL);
	Application->Window = this;
	Application->Init(t_hWnd);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(t_hWnd);
	ImGui_ImplDX11_Init(Application->Graphics.Device, Application->Graphics.Context);
	
	Application->Graphics.EndFrame();

	if (Rdoc) Rdoc->EndFrameCapture(NULL, NULL);
}

void WindowsWindow::Resize(uint32 width, uint32 height)
{
	SetWindowPos(hWnd, HWND_NOTOPMOST, WindowRect.left, WindowRect.top,
				 width, height,
				 SWP_FRAMECHANGED | SWP_NOACTIVATE);

	WindowRect.right = WindowRect.left + width;
	WindowRect.bottom = WindowRect.top + height;
	ShowWindow(hWnd, SW_NORMAL);

	Application->Width = (float32)width;
	Application->Height = (float32)height;
	Application->Resize();
	
}
	
void WindowsWindow::ToggleFullscreen()
{

	if(FullscreenMode)
	{
		SetWindowLong(hWnd, GWL_STYLE, WindowStyle);
		SetWindowPos(hWnd, HWND_NOTOPMOST, WindowRect.left, WindowRect.top,
					  WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top,
					  SWP_FRAMECHANGED | SWP_NOACTIVATE);
        ShowWindow(hWnd, SW_NORMAL);
	}
	else
	{
		GetWindowRect(hWnd, &WindowRect);
		SetWindowLong(hWnd, GWL_STYLE, WindowStyle & ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME));

		RECT fullscreenWindowRect;
		IDXGIOutput* Output;
		Application->Graphics.Swap->GetContainingOutput(&Output);
		DXGI_OUTPUT_DESC Desc;
		Output->GetDesc(&Desc);
		fullscreenWindowRect = Desc.DesktopCoordinates;
		Output->Release();

		SetWindowPos(hWnd, HWND_TOPMOST, fullscreenWindowRect.left, fullscreenWindowRect.top,
					 fullscreenWindowRect.right, fullscreenWindowRect.bottom,
					 SWP_FRAMECHANGED | SWP_NOACTIVATE);
        ShowWindow(hWnd, SW_MAXIMIZE);		
	}
	
	FullscreenMode = !FullscreenMode;
}	

void WindowsWindow::UpdateJoyStickState()
{
	XINPUT_STATE xState{ 0 };
	XInputGetState(0, &xState);

	JoystickInput newState;
	newState.Buttons = xState.Gamepad.wButtons;
	newState.LeftTrigger = xState.Gamepad.bLeftTrigger;
	newState.RightTrigger = xState.Gamepad.bRightTrigger;
	newState.LeftThumbX = xState.Gamepad.sThumbLX;
	newState.LeftThumbY = xState.Gamepad.sThumbLY;
	newState.RightThumbX = xState.Gamepad.sThumbRX;
	newState.RightThumbY = xState.Gamepad.sThumbRY;
		
	Input::gInput.UpdateJoystick(newState);
}

int WindowsWindow::Run()
{

	clock_t deltaTime = 0;
	clock_t dt = 0;
	unsigned int frames = 0;
	double  frameRate = 30;
	double  averageFrameTimeMilliseconds = 33.333;

	bool inGpuTiming = false;
	bool inGpuStats = false;

	while (true)
	{
		Input::gInput.Update();
		UpdateJoyStickState();
		
		MSG msg;
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				if(CleanDestroy) Deinit();
				return (int)msg.wParam;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (Input::gInput.IsKeyReleased(KeyCode::F11))
		{
			ToggleFullscreen();
		}

		if (!Minimized)
		{
			clock_t beginFrame = clock();
			
			

			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			ImGui::Begin("DirectXer");

			if(Application->Graphics.GetTimingResult(LastGpuTiming))
			{
				inGpuTiming = true;
				Application->Graphics.BeginTimingQuery();
			}

			if(Application->Graphics.GetStatisticsResult(LastGpuStats))
			{
				inGpuStats = true;
				Application->Graphics.BeginStatisticsQuery();
			}
			
			const float delta = (float)clockToMilliseconds(dt) / 1000.0f;
			Application->Update(delta);
			Application->Game.Update(delta);

			ImGui::End();

			ImGui::Render();
			if (Application->RenderImGui)
			{
				ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
			}
			

			if(inGpuTiming)
			{
				Application->Graphics.EndTimingQuery();
				inGpuTiming = false;
			}
			
			if(inGpuStats)
			{
				Application->Graphics.EndStatisticsQuery();
				inGpuStats = false;
			}
			
			Application->Graphics.EndFrame(Application->EnableVsync);

			clock_t endFrame = clock();

			dt = endFrame - beginFrame;
			deltaTime += dt;
			frames++;
			
			if (clockToMilliseconds(deltaTime) > 1000.0)
			{
				frameRate = (double)frames * 0.5 +  frameRate * 0.5;
				deltaTime -= CLOCKS_PER_SEC;
				averageFrameTimeMilliseconds = 1000.0 / (frameRate == 0 ? 0.001 : frameRate);

				DXLOG("[SYS] Frame time: {:.2} ms; FPS: {} ", averageFrameTimeMilliseconds, frames);

				LastFPS = (float)frames;
				LastFrameTime = (float)averageFrameTimeMilliseconds;
			
				frames = 0;
			}
			
		}
	}

	return 0;
		
}

void WindowsWindow::Deinit()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext(); 

	gDxgiManager.Destroy();
	Application->Graphics.Destroy();
	OPTICK_SHUTDOWN();
	
	UnregisterClass("DirectXer Window", GetModuleHandleA(NULL));
	FreeConsole();
	DestroyWindow(hWnd);
}

void WindowsWindow::HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
			  Minimized = true;
			  DXDEBUG("[Event] Minimizing");
			  break;
		  }

		  if(wParam == SIZE_MAXIMIZED || wParam == SIZE_RESTORED)
		  {
			  Minimized = false;
			  DXDEBUG("[Event] Maximizing");
		  }

		  uint32 width = LOWORD(lParam);
		  uint32 height = HIWORD(lParam);
		  
		  if (width == 0 || height == 0) break;

		  Application->Width = (float32)width;
		  Application->Height = (float32)height;
		  Application->Resize();
			  
		  DXDEBUG("[Event] Resizing: {}x{}", width, height);
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
		  Input::gInput.UpdateMouseScroll((float)delta);
		  break;
	  }
	  
	}
}

void SetupConsole()
{
	AllocConsole();

	FILE* fDummy;
	freopen_s(&fDummy, "CONOUT$", "w", stderr);
	freopen_s(&fDummy, "CONOUT$", "w", stdout);

}

void LogHResult(const char* t_File, uint32 t_Line, HRESULT t_Hr)
{
	auto errorString = DXGetErrorString(t_Hr);
	char errorDescription[512];
	DXGetErrorDescription(t_Hr, errorDescription, (DWORD)sizeof(errorDescription) );

	gLogger.PrintError(t_File, t_Line, "{} :", errorString, errorDescription);		 
}
