#include "Window.h"
#include <sstream>

Window::WindowClass Window::WindowClass::wndClass;

Window::Window(int width, int height, const char* name)
{
    RECT wr;
    wr.left = 100;
    wr.right = width + wr.left;
    wr.top = 100;
    wr.bottom = height + wr.top;

    if (AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE) == 0)
    {
        throw WND_LAST_EXCEPT();
    }

    hWnd = CreateWindow(
        WindowClass::wndClassName, name,
        WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
        nullptr, nullptr, WindowClass::GetInstance(), this
    );

    if (hWnd == nullptr)
    {
        throw WND_LAST_EXCEPT();
    }
    ShowWindow(hWnd, SW_SHOWDEFAULT);

}

Window::~Window()
{
    DestroyWindow(hWnd);
}

LRESULT Window::HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_NCCREATE)
    {
        const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
        Window* const pWnd = static_cast<Window*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
        SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::HandleMsgThunk));
        return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT Window::HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    Window* const pWnd = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
}

LRESULT Window::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
          m_Keyboard.ClearState();
          break;
      }

      case WM_KEYDOWN:
      case WM_SYSKEYDOWN:
      {
          if( !(lParam & 0x40000000) || m_Keyboard.AutorepeatIsEnabled() )
          {
              m_Keyboard.OnKeyPressed( static_cast<unsigned char>(wParam) );
          }
          break;
      }

      case WM_KEYUP:
      case WM_SYSKEYUP:
      {
          m_Keyboard.OnKeyReleased( static_cast<unsigned char>(wParam) );
          break;
      }

      case WM_CHAR:
      {
          m_Keyboard.OnChar( static_cast<unsigned char>(wParam) );
          break;
      }

      case WM_MOUSEMOVE:
      {
          const POINTS pt = MAKEPOINTS( lParam );

          if( pt.x >= 0 && pt.x < width && pt.y >= 0 && pt.y < height )
          {
              m_Mouse.OnMouseMove( pt.x,pt.y );
              if( !m_Mouse.IsInWindow() )
              {
                  SetCapture( hWnd );
                  m_Mouse.OnMouseEnter();
              }
          }

          else
          {
              if( wParam & (MK_LBUTTON | MK_RBUTTON) )
              {
                  m_Mouse.OnMouseMove( pt.x,pt.y );
              }
              // button up -> release capture / log event for leaving
              else
              {
                  ReleaseCapture();
                  m_Mouse.OnMouseLeave();
              }
          }
          break;
      }

      case WM_LBUTTONDOWN:
      {
          const POINTS pt = MAKEPOINTS( lParam );
          m_Mouse.OnLeftPressed( pt.x,pt.y );
          break;
      }

      case WM_RBUTTONDOWN:
      {
          const POINTS pt = MAKEPOINTS( lParam );
          m_Mouse.OnRightPressed( pt.x,pt.y );
          break;
      }

      case WM_LBUTTONUP:
      {
          const POINTS pt = MAKEPOINTS( lParam );
          m_Mouse.OnLeftReleased( pt.x,pt.y );
          // release m_Mouse if outside of window
          if( pt.x < 0 || pt.x >= width || pt.y < 0 || pt.y >= height )
          {
              ReleaseCapture();
              m_Mouse.OnMouseLeave();
          }
          break;
      }

      case WM_RBUTTONUP:
      {
          const POINTS pt = MAKEPOINTS( lParam );
          m_Mouse.OnRightReleased( pt.x,pt.y );
          // release m_Mouse if outside of window
          if( pt.x < 0 || pt.x >= width || pt.y < 0 || pt.y >= height )
          {
              ReleaseCapture();
              m_Mouse.OnMouseLeave();
          }
          break;
      }

      case WM_MOUSEWHEEL:
      {
          const POINTS pt = MAKEPOINTS( lParam );
          const int delta = GET_WHEEL_DELTA_WPARAM( wParam );
          m_Mouse.OnWheelDelta( pt.x,pt.y,delta );
          break;
      }


    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}


HINSTANCE Window::WindowClass::GetInstance()
{
    return wndClass.hInst;
}

Window::WindowClass::WindowClass() : hInst(GetModuleHandle(nullptr))
{
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(wc);
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = HandleMsgSetup;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = GetInstance();
    wc.hIcon = nullptr;
    wc.hCursor = nullptr;
    wc.hbrBackground = nullptr;
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = WindowClass::wndClassName;
    wc.hIconSm = nullptr;
    RegisterClassEx(&wc);
}

Window::WindowClass::~WindowClass()
{
    UnregisterClass(wndClassName, GetInstance());
}

std::optional<int> Window::ProcessMessages()
{
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

    return {};
}

void Window::SetTitle(const std::string& title)
{
    if(!SetWindowText(hWnd, title.c_str()))
    {
        throw WND_LAST_EXCEPT();
    }
}


Window::Exception::Exception( int line,const char* file,HRESULT hr ) noexcept
	: BaseException( line,file ),
	hr( hr )
{}

const char* Window::Exception::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] " << GetErrorCode() << std::endl
		<< "[Description] " << GetErrorString() << std::endl
		<< GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Window::Exception::GetType() const noexcept
{
	return "Window Exception";
}

std::string Window::Exception::TranslateErrorCode( HRESULT hr ) noexcept
{
	char* pMsgBuf = nullptr;
	// windows will allocate memory for err string and make our pointer point to it
	DWORD nMsgLen = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,hr,MAKELANGID( LANG_NEUTRAL,SUBLANG_DEFAULT ),
		reinterpret_cast<LPSTR>(&pMsgBuf),0,nullptr
	);
	// 0 string length returned indicates a failure
	if( nMsgLen == 0 )
	{
		return "Unidentified error code";
	}
	// copy error string from windows-allocated buffer to std::string
	std::string errorString = pMsgBuf;
	// free windows buffer
	LocalFree( pMsgBuf );
	return errorString;
}

HRESULT Window::Exception::GetErrorCode() const noexcept
{
	return hr;
}

std::string Window::Exception::GetErrorString() const noexcept
{
	return TranslateErrorCode( hr );
}
