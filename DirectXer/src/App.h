#pragma once

#include "Geometry.hpp"
#include "IncludeWin.h"
#include "Window.h"

class App
{
public:

	void Init(HWND t_Window);
	void Spin();

  private:
	Graphics m_Graphics;

	std::vector<float> plainGeometrVert;
	std::vector<size_t> plainGeometryInd;

	GeometryInfo cube;
	GeometryInfo plane;
	GeometryInfo sphere;

  public:
	int ReturnValue{0};
	boolean Running{true};

	
	
};
