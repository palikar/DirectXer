#pragma once

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include "Types.hpp"
#include "IncludeWin.hpp"
#include "Memory.hpp"

void SetupConsole();
	
struct WindowsSettings
{
	const char* ClassName;
	const char* WindowTitle;

	uint32 InitialWidth;
	uint32 InitialHeight;
	bool Fullscreen;

};

class App;
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
	static size_t FileSize(FileHandle handle);
	static void ReadFileIntoArena(FileHandle handle, size_t size, MemoryArena& t_Arena);
	static void CloseFile(FileHandle handle);
	
};
