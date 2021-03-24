#include "App.hpp"
#include "Glm.hpp"
#include "Input.hpp"
#include "Logging.hpp"
#include "Math.hpp"

#include <imgui.h>


static uint32 CUBE;
static uint32 PLANE;
static uint32 LINES;
static uint32 CYLINDER;
static uint32 SPHERE;
static uint32 AXIS;
static uint32 CAMERA;
static uint32 POINTLIGHT;
static uint32 SPOTLIGHT;

void App::Init(HWND t_Window)
{
	DXLOG("[RES] Resouces path: {}", Arguments.ResourcesPath.data());

	// @Todo: Refactor this into functions
	Graphics.initSwapChain(t_Window, Width, Height);
	Graphics.initBackBuffer();
	Graphics.initZBuffer(Width, Height);
	Graphics.initResources();
	Graphics.initRasterizationsStates();
	Graphics.initSamplers();
	Graphics.initBlending();

	Game.Application = this;
	Game.Graphics = &Graphics;
	Game.Init();
}

void App::Resize()
{
	Graphics.resizeBackBuffer(Width, Height);
	Graphics.destroyZBuffer();
	Graphics.initZBuffer(Width, Height);
	Graphics.setViewport(0, 0, Width, Height);
	
	Game.Resize();
}

void App::Destroy()
{
	Graphics.Destroy();
}

void App::Spin(float dt)
{
	Game.Update(dt);
}
