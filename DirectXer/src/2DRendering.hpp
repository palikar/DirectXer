#pragma once

#include "Types.hpp"
#include "Glm.hpp"
#include "GraphicsCommon.hpp"
#include "Graphics.hpp"
#include "ImageLibrary.hpp"
#include "FontLibrary.hpp"
#include "Resources.hpp"


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
	FontLibrary FontLib;

	uint32 CurrentVertexCount;

	inline static const size_t TotalVertices = 2000;
	inline static const uint8 MaxTextureSlots = 3;

	VBObject vbo;
	IBObject ibo;

	TextureObject TexSlots[MaxTextureSlots];
	uint8 CurrentTextureSlot;

	Graphics* Graph;
	Init2DParams Params;

  public:

	
	void InitRenderer(Graphics* t_Graphics, Init2DParams t_Params);

	void BeginScene();
	void EndScene();

	uint8 AttachTexture(TextureObject t_Tex);

	void DrawQuad(glm::vec2 pos, glm::vec2 size, glm::vec4 color);
	void DrawCirlce(glm::vec2 pos, float radius, glm::vec4 color);
	void DrawImage(uint32 t_Id, glm::vec2 pos, glm::vec2 size);
	void DrawRoundedQuad(glm::vec2 pos, glm::vec2 size, glm::vec4 color, float radius);
	void DrawText(std::string_view text, glm::vec2 pos, uint8 typeface)
	{
		if (CurrentVertexCount + 4 >= TotalVertices)
		{
			EndScene();
			BeginScene();
		}

		glm::vec2 currentPen{0.0f, 0.0f};
		for (const auto ch : text)
		{
			auto entry = FontLib.GetEntry(typeface, ch);

			auto rect = pos + currentPen - entry.GlyphSize;
			auto screenSize = glm::vec2{ (entry.Size.x) * 1024.0f, (entry.Size.y) * 1024.0f};
			currentPen += entry.Advance;
			if (ch == ' ') continue;

			auto slot = AttachTexture(entry.TexHandle);
			uint32 type = (slot << 8) | (5 << 0);

			CurrentVertex->pos = rect;
			CurrentVertex->color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
			CurrentVertex->uv = entry.Pos;
			CurrentVertex->type = type;
			++CurrentVertex;

			CurrentVertex->pos = rect + glm::vec2{screenSize.x, 0.0f};
			CurrentVertex->color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
			CurrentVertex->uv = entry.Pos + glm::vec2{entry.Size.x, 0.0f};
			CurrentVertex->type = type;
			++CurrentVertex;

			CurrentVertex->pos = rect + glm::vec2{0.0f, screenSize.y};
			CurrentVertex->uv = entry.Pos + glm::vec2{0.0f, entry.Size.y};
			CurrentVertex->color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
			CurrentVertex->type = type;
			++CurrentVertex;

			CurrentVertex->pos = rect + glm::vec2{screenSize.x, screenSize.y};
			CurrentVertex->color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
			CurrentVertex->uv = entry.Pos + glm::vec2{entry.Size.x, entry.Size.y};
			CurrentVertex->type = type;
			++CurrentVertex;

			Indices.insert(Indices.end(), { CurrentVertexCount , CurrentVertexCount + 1, CurrentVertexCount + 2,
					CurrentVertexCount + 2 , CurrentVertexCount + 1, CurrentVertexCount + 3});

			CurrentVertexCount += 4;
			
		}
	}
};

// Drawing rounded Quad

// Drawing Lines

// Pusing transform matrices
