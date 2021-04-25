#pragma once

#include <Types.hpp>
#include <Glm.hpp>
#include <GraphicsCommon.hpp>
#include <Graphics.hpp>
#include <ImageLibrary.hpp>
#include <FontLibrary.hpp>
#include <Resources.hpp>

struct Init2DParams
{
	float32 Width;
	float32 Height;
};

class Renderer2D
{
  public:

	BulkVector<Vertex2D> Vertices;
	BulkVector<uint32> Indices;
	Vertex2D* CurrentVertex;
	ImageLibrary ImageLib;
	FontLibrary FontLib;

	uint32 CurrentVertexCount;

	inline static const size_t TotalVertices = 2000;
	inline static const uint8 MaxTextureSlots = 3;

	VertexBufferId vbo;
	IndexBufferId ibo;

	TextureId TexSlots[MaxTextureSlots];
	uint8 CurrentTextureSlot;

	Graphics* Graph;
	Init2DParams Params;

  public:

	void InitRenderer(Graphics* t_Graphics, Init2DParams t_Params);

	void BeginScene();
	void EndScene();

	uint8 AttachTexture(TextureId t_Tex);

	void DrawQuad(glm::vec2 pos, glm::vec2 size, glm::vec4 color);
	void DrawCirlce(glm::vec2 pos, float radius, glm::vec4 color);
	void DrawImage(uint32 t_Id, glm::vec2 pos, glm::vec2 size);
	void DrawRoundedQuad(glm::vec2 pos, glm::vec2 size, glm::vec4 color, float radius);
	void DrawSubImage(uint32 t_Id, glm::vec2 pos, glm::vec2 size, glm::vec2 subPos, glm::vec2 subSize);
	void DrawText(std::string_view text, glm::vec2 pos, FontId typeface);
};

// Pusing transform matrices


struct SpriteSheetHolder
{
	struct SpriteSheet
	{
		glm::vec2 SubSize;
		glm::ivec2 GridSize;
		uint32 ImageIndex;
	};

	BulkVector<SpriteSheet> Sheets;
	Renderer2D* Gfx2D;


	void Init(size_t t_Size, Renderer2D* Gfx)
	{
		Sheets.reserve(t_Size);
		Gfx2D = Gfx;
	}

	uint32 PutSheet(uint32 t_ImageIndex, glm::vec2 t_Size, glm::ivec2 t_GridSize)
	{
		SpriteSheet sheet;
		sheet.SubSize = glm::vec2{ t_Size.x / t_GridSize.x, t_Size.y / t_GridSize.y };
		sheet.GridSize = t_GridSize;
		sheet.ImageIndex = t_ImageIndex;
		Sheets.push_back(sheet);

		return (uint32)Sheets.size() - 1;
	}

	void DrawSprite(size_t spiretSheet, int index, glm::vec2 pos, glm::vec2 size)
	{
		const auto& sheet = Sheets[spiretSheet];

		const int x = index % sheet.GridSize.x;
		const int y = index / sheet.GridSize.x;

		Gfx2D->DrawSubImage(sheet.ImageIndex, pos, size, { x * sheet.SubSize.x, y * sheet.SubSize.y }, sheet.SubSize);
	}

	void DrawSprite(uint32 spiretSheet, glm::ivec2 spirtePos, glm::vec2 pos, glm::vec2 size)
	{
		const auto& sheet = Sheets[spiretSheet];

		const int x = spirtePos.x;
		const int y = spirtePos.y;

		Gfx2D->DrawSubImage(sheet.ImageIndex, pos, size, { x * sheet.SubSize.x, y * sheet.SubSize.y }, sheet.SubSize);
	}
};
