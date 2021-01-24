#pragma once

#include "Geometry.hpp"
#include "IncludeWin.hpp"
#include "Graphics.hpp"
#include "Camera.hpp"

struct CommandLineSettings
{
	std::string_view ResourcesPath;
};


struct GeometryIndex
{
	uint32 BaseIndex;
	uint32 IndexOffset;
	uint32 IndexCount;
};

struct GeometryBuffer
{
	// @Todo: This shold be something that does not allocate!
	std::vector<GeometryInfo> Infos;

	void DrawGeometry(Graphics graphics, uint32 t_Index)
	{
		uint32 indexOffset = 0;
		uint32 baseIndex = 0;

		for (size_t j = 0; j < t_Index; ++j)
		{
			indexOffset += Infos[j].indexCount;
			baseIndex += Infos[j].vertexCount;
		}

		const uint32 indexCount = Infos[t_Index].indexCount;
		const auto topology = Graphics::TopolgyType((Infos[t_Index].type & ~255) >> 8);
		graphics.drawIndex(topology, indexCount, indexOffset, baseIndex);

	}

	uint32 PutGeometry(GeometryInfo t_Info)
	{
		Infos.push_back(t_Info);
		return (uint32)Infos.size() - 1;
	}

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
	GeometryBuffer geometryBuffer;
	Camera camera;

	RasterizationState CurrentRastState = RS_DEBUG;

	

  public:
	float32 Width;
	float32 Height;
	int ReturnValue{0};
	boolean Running{true};

	CommandLineSettings Arguments;

	
	
};
