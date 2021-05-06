#pragma once

#include <Geometry.hpp>
#include <Graphics.hpp>
#include <Camera.hpp>
#include <TextureCatalog.hpp>
#include <GeometryUtils.hpp>
#include <Materials.hpp>
#include <Lighting.hpp>
#include <2DRendering.hpp>
#include <Memory.hpp>
#include <GameDefinition.hpp>

struct CommandLineSettings
{
	std::string_view ResourcesPath;
};

/*
  @Note: The App handles anything that is shared between all games; it
  keeps and manages the shared state of each possible game; most
  notably, it initialized the Graphics and sets them up reay for
  rendering

*/

class App
{
public:
    // @Note: Called during the initialization of the platform layer's
    // window
	void Init(HWND t_Window);

    // @Note: Called when the window is fully initialized
	void PostInit();
    
	void Resize();

    // @Note: the upate method is called every frame by the platform layer
	void Update(float dt);
		
	// @Note: Application Data -- used by the "application" for
	// application management stuff
	float32 Width;
	float32 Height;
	int ReturnValue{0};
	boolean Running{true};
	CommandLineSettings Arguments;

	// @Note: Handle to the Platform's window
	PlatformLayer::WindowType* Window;

    // @Note: Rendering data -- used by the scene to
	// do its rendering
	Graphics Graphics;

	// @Note: Depending on how the project was build, this will
	// be a different "game"
	GameClass Game;

    // @Note: Keeping trackig of time so that we can do some things
    // only periodically
	float Timer;
};
