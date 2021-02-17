#pragma once

#include "Types.hpp"
#include "Glm.hpp"
#include "GraphicsCommon.hpp"
#include "Graphics.hpp"


static uint32 MaxVertices = 1000;

struct Rendering2DData
{
	std::vector<Vertex2D> Vertices;
	std::vector<uint32> Indices;

	VBObject vbo;
	IBObject ibo;
	

};

struct Init2DParams
{
	size_t t_Width;
	size_t t_Height;
}

void InitRenderer(Rendering2DData& t_Data, Init2DParams& t_Params)
{


}

// Drawing colored Quad
	
// Drawing textured Quad
	
// Drawing rounded Quad

// Drawing rounded textured Quad

// Drawing circle

// Drawing textured circle
