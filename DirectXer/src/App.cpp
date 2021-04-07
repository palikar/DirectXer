#include "App.hpp"
#include "Glm.hpp"
#include "Input.hpp"
#include "Logging.hpp"
#include "Math.hpp"

#include <imgui.h>

void App::Init(HWND t_Window)
{
	DXLOG("[RES] Resouces path: {}", Arguments.ResourcesPath.data());

	// @Todo: Refactor this into functions
	Graphics.InitSwapChain(t_Window, Width, Height);
	Graphics.InitBackBuffer();
	Graphics.InitZBuffer(Width, Height);
	Graphics.InitResources();
	Graphics.InitRasterizationsStates();
	Graphics.InitSamplers();
	Graphics.InitBlending();
	Graphics.InitDepthStencilStates();

	Game.Application = this;
	Game.Graphics = &Graphics;
	Game.Init();
}

void App::PostInit()
{
	// @Note: Windows is weird and it sends a resize mesage during the creation
	// of the window; this method will be called after everything is initialized
	// and no more spurious resize messages are expected to arrive
	Game.PostInit();
}

void App::Resize()
{
	Graphics.ResizeBackBuffer(Width, Height);
	Graphics.DestroyZBuffer();
	Graphics.InitZBuffer(Width, Height);
	Graphics.SetViewport(0, 0, Width, Height);
	
	Game.Resize();
}
