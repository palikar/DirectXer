
#include "IncludeWin.h"
#include "Input.h"
#include "Glm.h"
#include "App.h"

#include <fcntl.h>
#include <stdio.h>
#include <io.h>
#include <iostream>

void SetupConsole()
{

	AllocConsole();
	
    FILE* fDummy;
    freopen_s(&fDummy, "CONIN$", "r", stdin);
    freopen_s(&fDummy, "CONOUT$", "w", stderr);
    freopen_s(&fDummy, "CONOUT$", "w", stdout);

}



      // case WM_CLOSE:
      // {
      //     PostQuitMessage(0);
      //     return 0;
      // }

      // case WM_KILLFOCUS:
      // {
      //     m_Keyboard.ClearState();
      //     break;
      // }

      // case WM_KEYDOWN:
      // case WM_SYSKEYDOWN:
      // {
      //     if( !(lParam & 0x40000000) || m_Keyboard.AutorepeatIsEnabled() )
      //     {
      //         m_Keyboard.OnKeyPressed( static_cast<unsigned char>(wParam) );
      //     }
      //     break;
      // }

      // case WM_KEYUP:
      // case WM_SYSKEYUP:
      // {
      //     m_Keyboard.OnKeyReleased( static_cast<unsigned char>(wParam) );
      //     break;
      // }

      // case WM_CHAR:
      // {
      //     m_Keyboard.OnChar( static_cast<unsigned char>(wParam) );
      //     break;
      // }

      // case WM_MOUSEMOVE:
      // {
      //      const POINTS pt = MAKEPOINTS( lParam );

      //     if( pt.x >= 0 && pt.x < width && pt.y >= 0 && pt.y < height )
      //     {
      //         m_Mouse.OnMouseMove( pt.x,pt.y );
      //         if( !m_Mouse.IsInWindow() )
      //         {
      //             SetCapture( hWnd );
      //             m_Mouse.OnMouseEnter();
      //         }
      //     }

      //     else
      //     {
      //         if( wParam & (MK_LBUTTON | MK_RBUTTON) )
      //         {
      //             m_Mouse.OnMouseMove( pt.x,pt.y );
      //         }
      //         // button up -> release capture / log event for leaving
      //         else
      //         {
      //             ReleaseCapture();
      //             m_Mouse.OnMouseLeave();
      //         }
      //     }
      //     break;
      // }

      // case WM_LBUTTONDOWN:
      // {
      //     const POINTS pt = MAKEPOINTS( lParam );
      //     m_Mouse.OnLeftPressed( pt.x,pt.y );
      //     break;
      // }

      // case WM_RBUTTONDOWN:
      // {
      //     const POINTS pt = MAKEPOINTS( lParam );
      //     m_Mouse.OnRightPressed( pt.x,pt.y );
      //     break;
      // }

      // case WM_LBUTTONUP:
      // {
      //     const POINTS pt = MAKEPOINTS( lParam );
      //     m_Mouse.OnLeftReleased( pt.x,pt.y );
      //     // release m_Mouse if outside of window
      //     if( pt.x < 0 || pt.x >= width || pt.y < 0 || pt.y >= height )
      //     {
      //         ReleaseCapture();
      //         m_Mouse.OnMouseLeave();
      //     }
      //     break;
      // }

      // case WM_RBUTTONUP:
      // {
      //     const POINTS pt = MAKEPOINTS( lParam );
      //     m_Mouse.OnRightReleased( pt.x,pt.y );
      //     // release m_Mouse if outside of window
      //     if( pt.x < 0 || pt.x >= width || pt.y < 0 || pt.y >= height )
      //     {
      //         ReleaseCapture();
      //         m_Mouse.OnMouseLeave();
      //     }
      //     break;
      // }

      // case WM_MOUSEWHEEL:
      // {
      //     const POINTS pt = MAKEPOINTS( lParam );
      //     const int delta = GET_WHEEL_DELTA_WPARAM( wParam );
      //     m_Mouse.OnWheelDelta( pt.x,pt.y,delta );
      //     break;
      // }


    

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

	// SetWindowText(hWnd, title.c_str())

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

	 UnregisterClass("DirectXer Window", hInstance);
	 FreeConsole();
	 DestroyWindow(windowHanlde);
	return 0;
}
