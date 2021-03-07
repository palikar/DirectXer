#pragma once

#include "IncludeWin.hpp"
#include "Input.hpp"
#include "Glm.hpp"
#include "App.hpp"
#include "Types.hpp"
#include "Logging.hpp"

#include <fcntl.h>
#include <stdio.h>
#include <io.h>
#include <iostream>
#include <Stringapiset.h>
#include <shellapi.h>
#include <time.h>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#include <fmt/format.h>


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
