#pragma once

#include "Types.hpp"
#include "Glm.hpp"
#include "GraphicsCommon.hpp"
#include "Graphics.hpp"
#include "ImageLibrary.hpp"


struct Init2DParams
{
	float32 Width;
	float32 Height;
};

class Renderer2D
{
public:

	asl::BulkVector<Vertex2D> Vertices;
	asl::BulkVector<uint32> Indices;
	Vertex2D* CurrentVertex;
	ImageLibrary ImageLib;

	uint32 CurrentVertexCount;

	inline static const size_t TotalVertices = 2000;
	inline static const uint8 MaxTextureSlots = 3;

	VBObject vbo;
	IBObject ibo;

	TextureObject TexSlots[MaxTextureSlots];
	uint8 CurrentTextureSlot;

	Graphics* Graph;
	Init2DParams Params;

	void InitRenderer(Graphics* t_Graphics, Init2DParams t_Params)
	{
		Graph = t_Graphics;
		Params = t_Params;
		CurrentTextureSlot = 0;

		ImageLib.Init(t_Graphics);
			
		Vertices.resize(TotalVertices);
		Indices.resize(TotalVertices * 3);
		
		CurrentVertex = &Vertices[0];

		vbo = Graph->createVertexBuffer(sizeof(Vertex2D), nullptr, (uint32)(sizeof(Vertex2D) * TotalVertices), true);
		ibo = Graph->createIndexBuffer(nullptr, (uint32)(sizeof(uint32) * TotalVertices * 3), true);
	}

	void BeginScene()
	{
		CurrentVertexCount = 0;
		
		Indices.clear();
		Vertices.clear();

		CurrentVertex = &Vertices[0];		
	}

	void EndScene()
	{
		Graph->VertexShaderCB.projection = glm::transpose(glm::ortho(0.0f, Params.Width, Params.Height, 0.0f));
		Graph->VertexShaderCB.model = glm::mat4(1.0f);

		Graph->setShaderConfiguration(SC_2D_RECT);

		Graph->setIndexBuffer(ibo);
		Graph->setVertexBuffer(vbo);

		Graph->updateVertexBuffer(vbo, Vertices.data(), CurrentVertexCount * sizeof(Vertex2D));
		Graph->updateIndexBuffer(ibo, Indices.data(), 3u * CurrentVertexCount * sizeof(uint32));
		Graph->updateCBs();

		Graph->setBlendingState(BS_AlphaBlending);

		/*for (uint32 i = 0; i < MaxTextureSlots; i++)
		{
			Graph->bindTexture(i, TexSlots[i]);
		}*/
		Graph->bindTexture(0, TexSlots[0]);

		Graph->drawIndex(Graphics::TT_TRIANGLES, 3u * CurrentVertexCount, 0, 0);

	}

	uint8 AttachTexture(TextureObject t_Tex)
	{
		if(TexSlots[CurrentTextureSlot - 1].srv == t_Tex.srv)
		{
			return CurrentTextureSlot;
		}
			
		if (CurrentTextureSlot >= MaxTextureSlots)
		{
			EndScene();
			BeginScene();
		}
		TexSlots[CurrentTextureSlot++] = t_Tex;

		return CurrentTextureSlot - 1;
	}
	
	void DrawQuad(glm::vec2 pos, glm::vec2 size, glm::vec4 color)
	{
		if (CurrentVertexCount + 4 >= TotalVertices)
		{
			EndScene();
			BeginScene();
		}
		
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

		if (CurrentVertexCount + 4 >= TotalVertices)
		{
			EndScene();
			BeginScene();
		}
			
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

	void DrawImage(uint32 t_Id, glm::vec2 pos, glm::vec2 size)
	{
		if (CurrentVertexCount + 4 >= TotalVertices)
		{
			EndScene();
			BeginScene();
		}
		
		assert(t_Id < ImageLib.Images.size());
		const auto& screenImage = ImageLib.Images[t_Id];

		auto slot = AttachTexture(screenImage.TexHandle);
		uint32 type = (slot << 8) | (3 << 0);
		
		CurrentVertex->pos = pos;
		CurrentVertex->type = type;
		CurrentVertex->additional.x = screenImage.ScreenPos.x;
		CurrentVertex->additional.y = screenImage.ScreenPos.y;
		++CurrentVertex;

		CurrentVertex->pos = glm::vec2{pos.x + size.x, pos.y};
		CurrentVertex->type = type;
		CurrentVertex->additional.x = screenImage.ScreenPos.x + screenImage.ScreenSize.x;
		CurrentVertex->additional.y = screenImage.ScreenPos.y;
		++CurrentVertex;

		CurrentVertex->pos = glm::vec2{pos.x, pos.y + size.y};
		CurrentVertex->type = type;
		CurrentVertex->additional.x = screenImage.ScreenPos.x;
		CurrentVertex->additional.y = screenImage.ScreenPos.y + screenImage.ScreenSize.y;
		++CurrentVertex;

		CurrentVertex->pos = pos + size;
		CurrentVertex->type = type;
		CurrentVertex->additional.x = screenImage.ScreenPos.x + screenImage.ScreenSize.x;
		CurrentVertex->additional.y = screenImage.ScreenPos.y + screenImage.ScreenSize.y;
		++CurrentVertex;


		Indices.insert(Indices.end(), { CurrentVertexCount , CurrentVertexCount + 1, CurrentVertexCount + 2,
										CurrentVertexCount + 2 , CurrentVertexCount + 1, CurrentVertexCount + 3});

		CurrentVertexCount += 4;
		
	}

};

// Drawing rounded Quad

// Drawing Lines

// Pusing transform matrices
