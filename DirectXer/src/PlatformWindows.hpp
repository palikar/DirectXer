#pragma once

#include "Types.hpp"
#include "IncludeWin.hpp"

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


struct Platform
{
	static void* Allocate(size_t t_Size)
	{
		return VirtualAlloc(NULL, t_Size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	}
};
