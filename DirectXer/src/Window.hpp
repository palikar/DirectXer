#pragma once


#include "IncludeWin.hpp"
#include "Graphics.hpp"
#include "BaseException.hpp"
#include "Types.hpp"

#include <optional>
#include <string>
#include <string>
#include <memory>


class Window
{
public:

	class WindowClass
	{
	public:

		static HINSTANCE GetInstance();

		static constexpr const char* wndClassName = "DirectXer Window";
		HINSTANCE hInst;
		

	private:
		WindowClass();
		~WindowClass();
		WindowClass(const WindowClass&) = delete;
		WindowClass& operator=(const WindowClass&) = delete;

		
		
	};

    class Exception : public BaseException
    {
      public:
        Exception( int line,const char* file,HRESULT hr ) noexcept;

        const char* what() const noexcept override;
        virtual const char* GetType() const noexcept;
        static std::string TranslateErrorCode( HRESULT hr ) noexcept;
        HRESULT GetErrorCode() const noexcept;
        std::string GetErrorString() const noexcept;

      private:
        HRESULT hr;
    };


	Window(int width, int height, const char* name);
	~Window();
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;


	static std::optional<int> ProcessMessages();
	void SetTitle(const std::string& title);

	int width;
	int height;
	HWND hWnd;

};


#define WND_EXCEPT( hr ) Window::Exception( __LINE__,__FILE__,hr )
#define WND_LAST_EXCEPT() Window::Exception( __LINE__,__FILE__,GetLastError() )
