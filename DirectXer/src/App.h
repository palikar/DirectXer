#pragma once


#include "IncludeWin.h"
#include "Window.h"

class App
{
public:

	App();

	int Go();
	void processMessages();

	Window m_Window;
    Graphics m_Graphics;
	int m_ReturnValue{0};
	boolean m_Running{true};
};
