#pragma once

#include "Types.hpp"
#include "IncludeWin.hpp"
#include "Memory.hpp"

void SetupConsole();
	
LPSTR* CommandLineToArgvA(LPSTR lpCmdLine, INT* pNumArgs);

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
	void InitAfterCreate(HWND t_hWnd);
	void HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

};


struct PlatformLayer
{
	using FileHandle = HANDLE;
	
	static void* Allocate(size_t t_Size)
	{
		return VirtualAlloc(NULL, t_Size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	}

	static FileHandle OpenFileForReading(const char* t_Path)
	{
		FileHandle handle = CreateFile(t_Path, GENERIC_READ, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		return handle;
	}

	static size_t FileSize(FileHandle handle)
	{
		DWORD fileSize = GetFileSize(handle, NULL);
		return fileSize;
	}

	static void ReadFileIntoArena(FileHandle handle, size_t size, MemoryArena& t_Arena)
	{
		DWORD readBytes;
		ReadFile(handle, t_Arena.Memory + t_Arena.Size, (DWORD)size, &readBytes, NULL);
		assert(readBytes <= t_Arena.MaxSize - t_Arena.Size);
		t_Arena.Size += readBytes;
	}

	static FileHandle CloseFile(const char* t_Path, MemoryArena& t_Arena)
	{
		// @Todo: Probably queue this in some vector and close them in one go
		// at some point
		// CloseHandle(handle);
	}
	
	static void ReadWholeFile(const char* t_Path, MemoryArena& t_Arena)
	{

		HANDLE  handle = CreateFile(t_Path, GENERIC_READ, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		DWORD fileSize = GetFileSize(handle, NULL);
		DWORD readBytes;

		assert(fileSize < t_Arena.MaxSize);
		ReadFile(handle, t_Arena.Memory + t_Arena.Size, fileSize, &readBytes, NULL);

		t_Arena.Size += readBytes;

		CloseHandle(handle);
	}
};
