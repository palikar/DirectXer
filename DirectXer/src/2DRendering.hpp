#pragma once

#include "Types.hpp"
#include "Glm.hpp"
#include "GraphicsCommon.hpp"
#include "Graphics.hpp"
#include "Memory.hpp"


#include <stb_rect_pack.h>


struct Init2DParams
{
	float32 Width;
	float32 Height;
};


struct Image
{
	TextureObject TexHandle;
	glm::vec2 ScreenPos;
	glm::vec2 ScreenSize;

};

struct ImageAtlas
{
	TextureObject TexHandle;
	glm::vec2 Size;
	stbrp_context RectContext;
	

};



class ImageLibrary
{
  public:
	std::vector<Image> Images;
	std::vector<> Textures;

	MemoryArena fileArena = Memory::GetTempArena(Megabytes(16));

	void InitAtlas(glm::vec2 t_Size)
	{

	}

	uint32 LoadImage(std::string_view t_Resources, std::string_view t_Path)
	{
		fmt::basic_memory_buffer<char, 512> buf;
		fmt::format_to(buf, "{}/{}", t_Resources, t_Path);
		DXLOG("[RES] Loading {}", buf.c_str());
		ReadWholeFile(buf.c_str(), fileArena);
		int width, height, channels;
		unsigned char* data = stbi_load_from_memory((unsigned char*)fileArena.Memory, (int)fileArena.Size, &width, &height, &channels, 0);


	}


	Image GetImage(uint32 t_id)
	{
		return Images[t_id];
	}
	
};


class Renderer2D
{
public:

	asl::BulkVector<Vertex2D> Vertices;
	asl::BulkVector<uint32> Indices;
	Vertex2D* CurrentVertex;

	uint32 CurrentVertexCount;

	size_t TotalVertices = 2000;

	VBObject vbo;
	IBObject ibo;

	Graphics* Graph;
	Init2DParams Params;

	void InitRenderer(Graphics* t_Graphics, Init2DParams t_Params)
	{
		Graph = t_Graphics;
		Params = t_Params;

		
		Vertices.resize(TotalVertices);
		Indices.resize(TotalVertices * 3);
		
		CurrentVertex = &Vertices[0];

		vbo = Graph->createVertexBuffer(sizeof(Vertex2D), nullptr, (uint32)(sizeof(Vertex2D) * TotalVertices), true);
		ibo = Graph->createIndexBuffer(nullptr, (uint32)(sizeof(uint32) * TotalVertices * 3), true);
	}

	void BeginScene()
	{
		Graph->VertexShaderCB.projection = glm::transpose(glm::ortho(0.0f, Params.Width,  Params.Height, 0.0f));
		Graph->VertexShaderCB.model = glm::mat4(1.0f);

		CurrentVertexCount = 0;
		
		Indices.clear();
		Vertices.clear();

		CurrentVertex = &Vertices[0];		
	}

	void EndScene()
	{
		Graph->setShaderConfiguration(SC_2D_RECT);

		Graph->setIndexBuffer(ibo);
		Graph->setVertexBuffer(vbo);

		Graph->updateVertexBuffer(vbo, Vertices.data(), CurrentVertexCount*sizeof(Vertex2D));
		Graph->updateIndexBuffer(ibo, Indices.data(), 3u*CurrentVertexCount*sizeof(uint32));
		Graph->updateCBs();

		Graph->drawIndex(Graphics::TT_TRIANGLES, 3u * CurrentVertexCount, 0, 0);

	}
	
	void DrawQuad(glm::vec2 pos, glm::vec2 size, glm::vec4 color)
	{
		CurrentVertex->pos = pos;
		CurrentVertex->color = color;
		CurrentVertex->type = 1;
		++CurrentVertex;

		CurrentVertex->pos = glm::vec2{pos.x + size.x, pos.y};
		CurrentVertex->color = color;
		CurrentVertex->type = 1;
		++CurrentVertex;

		CurrentVertex->pos = glm::vec2{pos.x, pos.y + size.y};
		CurrentVertex->color = color;
		CurrentVertex->type = 1;
		++CurrentVertex;

		CurrentVertex->pos = pos + size;
		CurrentVertex->color = color;
		CurrentVertex->type = 1;
		++CurrentVertex;

		Indices.insert(Indices.end(), { CurrentVertexCount , CurrentVertexCount + 1, CurrentVertexCount + 2,
										CurrentVertexCount + 2 , CurrentVertexCount + 1, CurrentVertexCount + 3});

		CurrentVertexCount += 4;
		
	}

	void DrawCirlce(glm::vec2 pos, float radius, glm::vec4 color)
	{
		const float r2 = radius;
		
		CurrentVertex->pos = glm::vec2{pos.x - r2, pos.y - r2};
		CurrentVertex->color = color;
		CurrentVertex->additional = glm::vec3{radius, -1.0f, -1.0f};
		CurrentVertex->type = 2;
		++CurrentVertex;

		CurrentVertex->pos = glm::vec2{pos.x - r2, pos.y + r2};
		CurrentVertex->color = color;
		CurrentVertex->additional = glm::vec3{radius, -1.0f, 1.0f};
		CurrentVertex->type = 2;
		++CurrentVertex;

		CurrentVertex->pos = glm::vec2{pos.x + r2, pos.y - r2};
		CurrentVertex->color = color;
		CurrentVertex->additional = glm::vec3{radius, 1.0f, -1.0f};
		CurrentVertex->type = 2;
		++CurrentVertex;

		CurrentVertex->pos = glm::vec2{pos.x + r2, pos.y + r2};
		CurrentVertex->color = color;
		CurrentVertex->additional = glm::vec3{radius, 1.0f, 1.0f};
		CurrentVertex->type = 2;
		++CurrentVertex;

		Indices.insert(Indices.end(), { CurrentVertexCount , CurrentVertexCount + 1, CurrentVertexCount + 2,
										CurrentVertexCount + 2 , CurrentVertexCount + 1, CurrentVertexCount + 3});

		CurrentVertexCount += 4;
		
	}

};

// Drawing textured Quad
	
// Drawing rounded Quad

// Drawing rounded textured Quad

// Drawing textured circle


// Drawing Lines

// Pusing transform matrices?
