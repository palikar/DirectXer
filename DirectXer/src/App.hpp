#pragma once

#include "Geometry.hpp"
#include "IncludeWin.hpp"
#include "Graphics.hpp"
#include "Camera.hpp"

struct CommandLineSettings
{};

struct GeometryDraw
{
	uint32 Index{0};
	glm::mat4 Transform{1};
};

struct GeometryIndex
{
	uint32 BaseIndex;
	uint32 IndexOffset;
	uint32 IndexCount;
}

struct GeometryBuffer
{
	std::vector<GeometryInfo> Infos;

	std::vector<GeometryDraw> Draws;
};

static void ResetBuffer(GeometryBuffer& t_Buffer)
{
	t_Buffer.Draws.clear();
}


static void PutDraw(GeometryBuffer& t_Buffer, uint32 t_Index, glm::mat4 t_Transfrom)
{
	t_Buffer.Draws.push_back({t_Index, t_Transfrom});
}


static uint32 PutGeometry(GeometryBuffer& t_Buffer, GeometryInfo t_Info)
{
	t_Buffer.Infos.push_back(t_Info);
	return (uint32)t_Buffer.Infos.size();
}


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

	GeometryBuffer geometryBuffer;

	Camera camera;

  public:
	float32 Width;
	float32 Height;
	
	int ReturnValue{0};
	boolean Running{true};

	
	
};
