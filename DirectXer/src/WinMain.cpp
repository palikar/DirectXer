#include "IncludeWin.h"
#include "Input.h"
#include "Glm.h"
#include "App.h"
#include "Types.h"
#include "Logging.h"

#include <fcntl.h>
#include <stdio.h>
#include <io.h>
#include <iostream>

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

      case WM_CLOSE:
      {
          PostQuitMessage(0);
          return 0;
      }

      case WM_KILLFOCUS:
      {
          break;
      }

      case WM_KEYDOWN:
	  case WM_SYSKEYDOWN:
	  {
		  if( !(lParam & 0x40000000) )
		  {
			  gInput.UpdateKeyboardButtonPressed( static_cast<uint16_t>(wParam));
		  }
          break;
      }

      case WM_KEYUP:
      case WM_SYSKEYUP:
	  {
		  gInput.UpdateKeyboardButtonReleased( static_cast<uint16_t>(wParam));
          break;
      }

      // case WM_CHAR:
      // {
      //     break;
      // }

      case WM_MBUTTONUP:
	  {
		  auto mouseBtn = (unsigned int) (wParam & 3);
		  gInput.UpdateMouseButtonReleased(mouseBtn);
	  }

      case WM_MBUTTONDOWN:
	  {
		  auto mouseBtn = (unsigned int) (wParam & 3);
		  gInput.UpdateMouseButtonPressed(mouseBtn);
	  }

      case WM_MOUSEMOVE:
      {
		  const POINTS pt = MAKEPOINTS( lParam );
		  gInput.UpdateMousePos({pt.x, pt.y});
		  break;
      }
	  
      // case WM_LBUTTONDOWN:
      // {
	  // 	  gInput.UpdateMouseLeftPressed();
      //     break;
      // }

      // case WM_RBUTTONDOWN:
      // {
	  // 	  gInput.UpdateMouseRightPressed();
      //     break;
      // }

      // case WM_LBUTTONUP:
      // {
	  // 	  gInput.UpdateMouseLeftReleased();
      //     break;
      // }

      // case WM_RBUTTONUP:
      // {
	  // 	  gInput.UpdateMouseLeftReleased();
      //     break;
      // }

      case WM_MOUSEWHEEL:
      {
		  const POINTS pt = MAKEPOINTS( lParam );
		  const int delta = GET_WHEEL_DELTA_WPARAM( wParam );
		  gInput.UpdateMouseScroll((float)delta);
          break;
      }


    }

    return DefWindowProc(hWnd, msg, wParam, lParam);

}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdSHow)
{

    SetupConsole();
	gInput.Init();

	WNDCLASSEX windowClass{0};
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

    int INITIAL_WIDTH = 800;
    int INITIAL_HEIHT = 600;


	RECT windowRect;
    windowRect.left = 100;
    windowRect.right = INITIAL_WIDTH + windowRect.left;
    windowRect.top = 100;
    windowRect.bottom = INITIAL_HEIHT + windowRect.top;

	if (AdjustWindowRect(&windowRect, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE) == 0)
    {
        return 0;
    }

	App application;

    auto windowHanlde = CreateWindow(
        "DirectXer Window", "DirectXer",
        WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
        nullptr, nullptr, hInstance, &application
    );

	
    if (windowHanlde == nullptr)
    {
        return 0;
    }

	application.Init(windowHanlde);
	
	ShowWindow(windowHanlde, SW_SHOWDEFAULT);

	//SetWindowText(hWnd, title.c_str())

	DXLOG("This is pretty cool, not gonna lie!: {}", 213);
	DXLOG("This is pretty cool, not gonna lie!: {}", 213);
	DXERROR("This is pretty cool, not gonna lie!: {}", 213);
	DXLOG("This is pretty cool, not gonna lie!: {}", 213);
	DXERROR("This is pretty cool, not gonna lie!: {}", 213);
	DXPRINT("This is pretty cool, not gonna lie!: {}", 213);
	
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

	application.Destroy();
	UnregisterClass("DirectXer Window", hInstance);
	FreeConsole();
	DestroyWindow(windowHanlde);
	return 0;
}
