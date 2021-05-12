#pragma once

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <cassert>
#include <dxerr.h>
#include <dxgidebug.h>
#include <type_traits>

#pragma comment(lib, "dxguid.lib")

#include <Types.hpp>
#include <Utils.hpp>
#include <GraphicsCommon.hpp>

#include "IncludeWin.hpp"

void SetupConsole();
void LogHResult(const char* t_File, uint32 t_Line, HRESULT t_Hr);
	
struct WindowsSettings
{
	const char* ClassName;
	const char* WindowTitle;

	uint32 InitialWidth;
	uint32 InitialHeight;
	bool Fullscreen;
};

class App;
struct MemoryArena;
class WindowsWindow
{
public:

	App* Application;
	WindowsSettings Settings;
	HWND hWnd;
	bool FullscreenMode;
	UINT WindowStyle;
	RECT WindowRect;
	bool Minimized;

	GPUTimingResult LastGpuTiming;
	GPUStatsResult LastGpuStats;
	float LastFPS;
	float LastFrameTime;
	
	void Init(WindowsSettings t_Settings);
	void ToggleFullscreen();
	int Run();
	void Deinit();
	void Resize(uint32 width, uint32 height);
	void InitAfterCreate(HWND t_hWnd);
	void HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void UpdateJoyStickState();
};

struct WindowsPlatformLayer
{
	using FileHandle = HANDLE;
	using WindowType = WindowsWindow;

	inline static HANDLE StdOutHandle;
	inline static HANDLE ErrOutHandle;

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

	static void Init();
	static void SetOuputColor(ConsoleForeground color);
	static void WriteStdOut(const char* data, size_t len);
	static void WriteErrOut(const char* data, size_t len);
	static void* Allocate(size_t t_Size);
	static FileHandle OpenFileForReading(const char* t_Path);
	static FileHandle OpenFileForWriting(const char* t_Path);
	static size_t FileSize(FileHandle handle);
	static void ReadFileIntoArena(FileHandle handle, size_t size, MemoryArena& t_Arena);
	static void WriteArenaIntoFile(FileHandle handle, MemoryArena& t_Arena);
	static void CloseFile(FileHandle handle);
	static bool IsValidPath(const char* data);
	static uint64 Clock();

	
};

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

#define DXLOGHRESULT(hr) LogHResult(__FILE__, __LINE__, hr)
