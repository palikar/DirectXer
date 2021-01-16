#pragma once

#include "Geometry.hpp"
#include "IncludeWin.hpp"
#include "Graphics.hpp"
#include "Camera.hpp"

struct CommandLineSettings
{
};

class App
{
public:

	void Init(HWND t_Window);
	void Spin();

	void Destroy();

	void Resize();

  private:
	Graphics Graphics;

	GeometryInfo axisHelper;

	Camera camera;

  public:
	float32 Width;
	float32 Height;
	
	int ReturnValue{0};
	boolean Running{true};

	
	
};
