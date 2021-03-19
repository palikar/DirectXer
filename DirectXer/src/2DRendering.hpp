#pragma once

#include "Types.hpp"
#include "Glm.hpp"
#include "GraphicsCommon.hpp"
#include "Graphics.hpp"
#include "ImageLibrary.hpp"
#include "Resources.hpp"

#include <robin_hood.h>

#include <ft2build.h>
#include FT_FREETYPE_H

inline float From26dot6ToAbsolute(FT_Pos f26dot6Value)
{
	return float(f26dot6Value) / 64.0f;
}


class FontLibrary
{
public:
	inline static const uint16 AtlasSize = 1024u;
	const static inline uint16 RectsCount = 1024u / 2u;
	const static inline uint16 Padding = 2;

	inline static const std::string_view Characters = "qwertzuiopasdfghjklyxcvbnm QWERTZUIOPASDFGHJKLYXCVBNM1234567890°!\"§$%&/(){}[]=?`´'><|,.-;:_#+*~öäüÖÄÜ";

	struct AtlasEntry
	{
		glm::vec2 Pos;
		glm::vec2 Size;
		float offset;
		TextureObject TexHandle;
		float Top;
		float Left;
		glm::vec2 Advance;		
	};

	FT_Library FTLibrary;
	std::vector<FT_Face> Faces;
	Graphics* Gfx;
	std::vector<TextureObject> Atlases;
	stbrp_context RectContext;
	robin_hood::unordered_map<char, AtlasEntry> AtlasGlyphEntries;

	void Init(Graphics* t_Graphics)
	{
		FT_Init_FreeType(&FTLibrary);
		Gfx = t_Graphics;

		InitNewAtlas();
	}

	void InitNewAtlas()
	{
		auto tex = Gfx->createTexture(AtlasSize, AtlasSize, TF_R, nullptr, 0);

		auto space = Memory::BulkGet<stbrp_node>(RectsCount);
		stbrp_init_target(&RectContext, AtlasSize, AtlasSize, space,  RectsCount);

		Atlases.push_back(tex);			
	}

	void LoadTypeface(std::string_view t_Path)
	{
		auto path = Resources::ResolveFilePath(t_Path);

		FT_Face face;
		FT_New_Face( FTLibrary, path, 0, &face);


		for (const auto ch : Characters)
		{
			auto glyph_index = FT_Get_Char_Index( face, (unsigned long)ch );

			FT_Set_Pixel_Sizes(face, 0, FT_UInt(32));
			
			if (FT_Load_Glyph(face, glyph_index, FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP))
			{
				DXERROR("Can't load glyph");
			}

			if(FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL))
			{
				DXERROR("Can't render glyph");
			}

			const auto& bitmap = face->glyph->bitmap;
			auto width = bitmap.width + Padding;
			auto height = bitmap.rows + Padding;

			stbrp_rect rect;
			rect.w = (stbrp_coord)width;
			rect.h = (stbrp_coord)height;

			stbrp_pack_rects(&RectContext, &rect, 1);

			Gfx->updateTexture(Atlases.back(), { {rect.x + Padding, rect.y + Padding}, { bitmap.width, bitmap.rows}}, bitmap.buffer, 1);

			AtlasEntry entry;
			entry.Pos = glm::vec2{rect.x + Padding, rect.y + Padding} / (float)AtlasSize;
			entry.Size = glm::vec2{ bitmap.width + Padding, bitmap.rows + Padding} / (float)AtlasSize;
			entry.TexHandle = Atlases.back();
			entry.Left = float(-face->glyph->bitmap_left);
			entry.Top = float(face->glyph->bitmap_top);
			entry.Advance = glm::vec2(From26dot6ToAbsolute(face->glyph->advance.x), From26dot6ToAbsolute(face->glyph->advance.y));

			AtlasGlyphEntries.insert({ch, entry });
		}
		
	}
};


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
	void DrawText(std::string_view text, glm::vec2 pos)
	{
		if (CurrentVertexCount + 4 >= TotalVertices)
		{
			EndScene();
			BeginScene();
		}

		// pos.y -= 32.0f;
		glm::vec2 currentPen{0.0f, 0.0f};
		for (const auto ch : text)
		{
			auto& entry = FontLib.AtlasGlyphEntries.at(ch);

			auto rect = pos - (-currentPen + glm::vec2{ entry.Left, entry.Top });
			auto screenSize = glm::vec2{ (entry.Size.x - 2.0f / 1024.0f) * 1024.0f, (entry.Size.y - 2.0f / 1024.0f) * 1024.0f};
			//screenSize -= glm::vec2(2.0f, 2.0f);

			auto slot = AttachTexture(entry.TexHandle);
			uint32 type = (slot << 8) | (5 << 0);

			currentPen += entry.Advance;
				
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
