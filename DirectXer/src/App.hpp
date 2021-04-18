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

class App
{
public:
	void Init(HWND t_Window);
	void PostInit();
	void Resize();

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
};
