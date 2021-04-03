#include "PlatformWindows.hpp"
#include "App.hpp"
#include "Input.hpp"
#include "Glm.hpp"
#include "Logging.hpp"

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <fmt/format.h>
#include <Xinput.h>

void PlatformLayer::Init()
{
	StdOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	ErrOutHandle = GetStdHandle(STD_ERROR_HANDLE);
}

void PlatformLayer::SetOuputColor(ConsoleForeground color)
{
	SetConsoleTextAttribute(StdOutHandle, (WORD)color);
}
		
void PlatformLayer::WriteStdOut(const char* data, size_t len)
{
	WriteFile(StdOutHandle, data, (DWORD)len, NULL, NULL);
}

void PlatformLayer::WriteErrOut(const char* data, size_t len)
{
	WriteFile(ErrOutHandle, data, (DWORD)len, NULL, NULL);
}
	
void* PlatformLayer::Allocate(size_t t_Size)
{
	return VirtualAlloc(NULL, t_Size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

PlatformLayer::FileHandle PlatformLayer::OpenFileForReading(const char* t_Path)
{
	FileHandle handle = CreateFile(t_Path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE)
	{
		DXERROR("Can't open file: {} ", t_Path);
	}
	return handle;
}

size_t PlatformLayer::FileSize(FileHandle handle)
{
	DWORD fileSize = GetFileSize(handle, NULL);
	return fileSize;
}

void PlatformLayer::ReadFileIntoArena(FileHandle handle, size_t size, MemoryArena& t_Arena)
{
	DWORD readBytes;
	ReadFile(handle, t_Arena.Memory + t_Arena.Size, (DWORD)size, &readBytes, NULL);
	assert(readBytes <= t_Arena.MaxSize - t_Arena.Size);
	t_Arena.Size += readBytes;
}

void PlatformLayer::CloseFile(FileHandle handle)
{
	// @Todo: Probably queue this in some vector and close them in one go
	// at some point
	// CloseHandle(handle);
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

void WindowsWindow::Init(WindowsSettings t_Settings)
{
	Settings = t_Settings;
	FullscreenMode = false;
	Minimized = false;

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

	assert(AdjustWindowRect(&WindowRect, WindowStyle, FALSE) != 0);
		
	Application->Width = (float32)Settings.InitialWidth;
	Application->Height = (float32)Settings.InitialHeight;

	hWnd = CreateWindow(t_Settings.ClassName, t_Settings.WindowTitle, WindowStyle,
						CW_USEDEFAULT, CW_USEDEFAULT, WindowRect.right - WindowRect.left,
						WindowRect.bottom - WindowRect.top,
						nullptr, nullptr, GetModuleHandleA(NULL), this);
	assert(hWnd);

	ShowWindow(hWnd, SW_SHOW);

	Application->PostInit();
}

void WindowsWindow::InitAfterCreate(HWND t_hWnd)
{
	Application->Window = this;
	Application->Init(t_hWnd);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(Application->Graphics.Device, Application->Graphics.Context);

	Application->Graphics.EndFrame();
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

	while (true)
	{
		Input::gInput.Update();
		UpdateJoyStickState();
		
		MSG msg;
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				Deinit();
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

			Application->Spin((float)clockToMilliseconds(dt) / 1000.0f);

			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); 

			Application->Graphics.EndFrame();

			clock_t endFrame = clock();

			dt = endFrame - beginFrame;
			deltaTime += endFrame - beginFrame;
			frames++;

			if( clockToMilliseconds(deltaTime)>1000.0)
			{
				frameRate = (double)frames * 0.5 +  frameRate * 0.5;
				deltaTime -= CLOCKS_PER_SEC;
				averageFrameTimeMilliseconds  = 1000.0/(frameRate ==0 ? 0.001 : frameRate);

				DXLOG("[SYS] Frame time: {:.2} ms; FPS: {} ", averageFrameTimeMilliseconds, frames);
			
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
	Application->Destroy();
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
			  DXLOG("[EVENT] Minimizing");
			  break;
		  }

		  if(wParam == SIZE_MAXIMIZED || wParam == SIZE_RESTORED)
		  {
			  Minimized = false;
			  DXLOG("[EVENT] Maximizing");
		  }

		  uint32 width = LOWORD(lParam);
		  uint32 height = HIWORD(lParam);
		  
		  if (width == 0 || height == 0) break;

		  Application->Width = (float32)width;
		  Application->Height = (float32)height;
		  Application->Resize();
			  
		  DXLOG("[EVENT] Resizing: {}x{}", width, height);
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
	freopen_s(&fDummy, "CONIN$", "r", stdin);
	freopen_s(&fDummy, "CONOUT$", "w", stderr);
	freopen_s(&fDummy, "CONOUT$", "w", stdout);

}
