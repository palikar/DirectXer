#pragma once

#include "Types.hpp"
#include "Memory.hpp"
#include "Platform.hpp"

#include <fmt/format.h>
#include <fmt/color.h>

#include <cassert>
#include <dxerr.h>
#include <dxgidebug.h>

#pragma comment(lib, "dxguid.lib")

struct Logger
{
	fmt::basic_memory_buffer<char, Kilobytes(1)> formatBuffer;
	
    template<typename ... Args>
    void PrintLog(const char* t_File, uint32 t_Line, const char* t_Format, Args ... t_Args)
	{
		formatBuffer.clear();
		fmt::format_to(formatBuffer, "[{}:{}] ", strrchr(t_File, '\\') + 1, t_Line);
		fmt::format_to(formatBuffer, t_Format, t_Args ... );
		fmt::format_to(formatBuffer, "\n");

		PlatformLayer::SetOuputColor(PlatformLayer::ConsoleForeground::WHITE);
		PlatformLayer::WriteStdOut(formatBuffer.data(), formatBuffer.size());
	}

	template<typename ... Args>
    void PrintError(const char* t_File, uint32 t_Line, const char* t_Format, Args ... t_Args)
	{
		formatBuffer.clear();

		fmt::format_to(formatBuffer, "[{}:{}] ", strrchr(t_File, '\\') + 1, t_Line);
		fmt::format_to(formatBuffer, t_Format, t_Args ... );
		fmt::format_to(formatBuffer, "\n");

		PlatformLayer::SetOuputColor(PlatformLayer::ConsoleForeground::RED);
		PlatformLayer::WriteErrOut(formatBuffer.data(), formatBuffer.size());
	}
	
    template<typename ... Args>
    void Print(const char* t_Format, Args ... t_Args)
	{
		formatBuffer.clear();

		fmt::format_to(formatBuffer, t_Format, t_Args ... );
		
		PlatformLayer::SetOuputColor(PlatformLayer::ConsoleForeground::WHITE);
		PlatformLayer::WriteStdOut(formatBuffer.data(), formatBuffer.size());
	}

	void LogHResult(const char* t_File, uint32 t_Line, HRESULT t_Hr)
	{
		auto errorString = DXGetErrorString(t_Hr);
		char errorDescription[512];
		DXGetErrorDescription(t_Hr, errorDescription, (DWORD)sizeof(errorDescription) );

		PrintError(t_File, t_Line, "{} :", errorString, errorDescription);		 
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
#define DXLOG(MSG, ...)  gLogger.PrintLog(__FILE__, __LINE__, MSG, __VA_ARGS__)
#define DXPRINT(MSG, ...)

#endif 



