#pragma once

#include "Types.hpp"
#include "IncludeWin.hpp"

#include <fmt/format.h>
#include <fmt/color.h>

#include <cassert>

#include <dxerr.h>
#include <dxgidebug.h>

#pragma comment(lib, "dxguid.lib")

struct Logger
{

	enum class ConsoleForeground : DWORD
	{
		BLACK = 0,
		DARKBLUE = FOREGROUND_BLUE,
		DARKGREEN = FOREGROUND_GREEN,
		DARKCYAN = FOREGROUND_GREEN | FOREGROUND_BLUE,
		DARKRED = FOREGROUND_RED,
		DARKMAGENTA = FOREGROUND_RED | FOREGROUND_BLUE,
		DARKYELLOW = FOREGROUND_RED | FOREGROUND_GREEN,
		DARKGRAY = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
		GRAY = FOREGROUND_INTENSITY,
		BLUE = FOREGROUND_INTENSITY | FOREGROUND_BLUE,
		GREEN = FOREGROUND_INTENSITY | FOREGROUND_GREEN,
		CYAN = FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE,
		RED = FOREGROUND_INTENSITY | FOREGROUND_RED,
		MAGENTA = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE,
		YELLOW = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN,
		WHITE = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
	};

	// @Imporve : Get everthing needed upfront
	// @Imporve : Buffer the thing a little bit
	// @Imporve : Add logging severities support
	// @Imporve : Use the platform layer for writing to STD out
	
    template<typename ... Args>
    void PrintLog(const char* t_File, uint32 t_Line, const char* t_Format, Args ... t_Args)
	{

		auto hOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hOutHandle, (DWORD)ConsoleForeground::WHITE);
		fmt::basic_memory_buffer<char, 512> out;
		fmt::format_to(out, "[{}:{}] ", strrchr(t_File, '\\') + 1, t_Line);
		fmt::format_to(out, t_Format, t_Args ... );
		fmt::format_to(out, "\n");
		
		WriteFile(hOutHandle, out.data(), (DWORD)out.size(), NULL, NULL);
	}

	template<typename ... Args>
    void PrintError(const char* t_File, uint32 t_Line, const char* t_Format, Args ... t_Args)
	{

		auto hOutHandle = GetStdHandle(STD_ERROR_HANDLE );
		SetConsoleTextAttribute(hOutHandle, (DWORD)ConsoleForeground::RED);
		
		fmt::basic_memory_buffer<char, 512> out;
		fmt::format_to(out, "[{}:{}] ", strrchr(t_File, '\\') + 1, t_Line);
		fmt::format_to(out, t_Format, t_Args ... );
		fmt::format_to(out, "\n");
		
		WriteFile(hOutHandle, out.data(), (DWORD)out.size(), NULL, NULL);
	}
	
    template<typename ... Args>
    void Print(const char* t_Format, Args ... t_Args)
	{

		auto hOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hOutHandle, (DWORD)ConsoleForeground::WHITE);
		fmt::basic_memory_buffer<char, 512> out;
		fmt::format_to(out, t_Format, t_Args ... );
		WriteFile(hOutHandle, out.data(), (DWORD)out.size(), NULL, NULL);
	}

	void LogHResult(const char* t_File, uint32 t_Line, HRESULT t_Hr)
	{

		auto errorString = DXGetErrorString(t_Hr);
		char errorDescription[512];
		DXGetErrorDescription(t_Hr, errorDescription, (DWORD)sizeof(errorDescription) );

		PrintError(t_File, t_Line, "{} :", errorString, errorDescription);

		// DWORD nMsgLen = FormatMessage(
		// 	FORMAT_MESSAGE_ALLOCATE_BUFFER |
		// 	FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		// 	nullptr,hr,MAKELANGID( LANG_NEUTRAL,SUBLANG_DEFAULT ),
		// 	reinterpret_cast<LPSTR>(&pMsgBuf),0,nullptr);
		// LocalFree( pMsgBuf );
		 
	}
    
};

inline Logger gLogger;

class DxgiInfoManager
{
  public:
	void Init()
	{
		typedef HRESULT (WINAPI* DXGIGetDebugInterface)(REFIID,void **);

		const auto hModDxgiDebug = LoadLibraryEx( "dxgidebug.dll",nullptr,LOAD_LIBRARY_SEARCH_SYSTEM32 );
		assert(hModDxgiDebug);
			
		const auto DxgiGetDebugInterface = reinterpret_cast<DXGIGetDebugInterface>(
			reinterpret_cast<void*>(GetProcAddress( hModDxgiDebug,"DXGIGetDebugInterface" ))
		);
		assert(( DxgiGetDebugInterface != nullptr ));
		DxgiGetDebugInterface( __uuidof(IDXGIInfoQueue),reinterpret_cast<void**>(&pDxgiInfoQueue) );

		assert(pDxgiInfoQueue);
	}

	void ResetMessage()
	{
		assert(pDxgiInfoQueue);
		next = pDxgiInfoQueue->GetNumStoredMessages( DXGI_DEBUG_ALL );
	}

	void Destroy()
	{
		assert(pDxgiInfoQueue);
		pDxgiInfoQueue->Release();
	}
	
	void PrintMessages()
	{
		const auto end = pDxgiInfoQueue->GetNumStoredMessages( DXGI_DEBUG_ALL );

		for( auto i = next; i < end; i++ )
		{
			SIZE_T messageLength;
		
			pDxgiInfoQueue->GetMessage(DXGI_DEBUG_ALL,i,nullptr,&messageLength);

			// @Improve : This will eventually cause a problem
			auto bytes = (DXGI_INFO_QUEUE_MESSAGE*)alloca(messageLength);
			pDxgiInfoQueue->GetMessage( DXGI_DEBUG_ALL,i, bytes, &messageLength );
			fmt::print("\t {} : {}\n", i - next, bytes->pDescription);
			
	}

	}
	
  private:
	unsigned long long next = 0u;
	struct IDXGIInfoQueue* pDxgiInfoQueue = nullptr;
};

inline DxgiInfoManager gDxgiManager;


#ifdef _DEBUG

#define DXERROR(MSG, ...) gLogger.PrintError(__FILE__, __LINE__, MSG, __VA_ARGS__)
#define DXLOG(MSG, ...) gLogger.PrintLog(__FILE__, __LINE__, MSG, __VA_ARGS__)
#define DXPRINT(MSG, ...) gLogger.Print(MSG, __VA_ARGS__)


#define DXLOGHRESULT(hr) gLogger.LogHResult(__FILE__, __LINE__, hr)

#else

#define DXERROR(MSG, ...)
#define DXLOG(MSG, ...) 
#define DXPRINT(MSG, ...)

#endif 



