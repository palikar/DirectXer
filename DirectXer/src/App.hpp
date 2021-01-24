#pragma once

#include "Geometry.hpp"
#include "IncludeWin.hpp"
#include "Graphics.hpp"
#include "Camera.hpp"
#include "TextureCatalog.hpp"
#include "GeometryUtils.hpp"

struct CommandLineSettings
{
	std::string_view ResourcesPath;
};

class App
{
public:

	void Init(HWND t_Window);
	void Spin();
	void Destroy();
	void Resize();

	
	Graphics Graphics;
	BufferDescriptor DebugGeometry;
	Camera camera;
	RasterizationState CurrentRastState = RS_DEBUG;

	float32 Width;
	float32 Height;
	int ReturnValue{0};
	boolean Running{true};
	CommandLineSettings Arguments;
	TextureCatalog Textures;
	
};
