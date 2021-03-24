#pragma once

#include "Types.hpp"
#include "Glm.hpp"
#include "GraphicsCommon.hpp"
#include "Graphics.hpp"
#include "Platform.hpp"

#include <robin_hood.h>
#include <stb_rect_pack.h>
#include <ft2build.h>
#include FT_FREETYPE_H

inline float ToFloat(FT_Pos fixed)
{
	return (float)fixed / 64.0f;
}

struct FontBuilder
{
	struct FontLoadEntry
	{
		std::string_view Path;
		float Size;
		PlatformLayer::FileHandle Handle;
		size_t FileSize;
	};
	
	TempVector<FontLoadEntry> LoadEntries;
	size_t MaxFileSize;

	void Init(size_t t_Size);
	size_t PutTypeface(std::string_view t_Path, float t_Size);
};

class FontLibrary
{
public:
	inline static const uint16 AtlasSize = 1024u;
	const static inline uint16 RectsCount = 1024u / 2u;
	const static inline uint16 Padding = 2;

	inline static const std::string_view Characters =
		"qwertzuiopasdfghjklyxcvbnm"
		"QWERTZUIOPASDFGHJKLYXCVBNM"
		"1234567890"
		"°!\"§$%&/(){}[]=?`´'><|,.-;:_#+*~öäüÖÄÜ";

	struct AtlasEntry
	{
		glm::vec2 Pos;
		glm::vec2 Size;
		TextureObject TexHandle;
		glm::vec2 GlyphSize;
		glm::vec2 Advance;		
	};

	FT_Library FTLibrary;
	Graphics* Gfx;
	BulkVector<TextureObject> Atlases;
	stbrp_context RectContext;
	BulkVector<AtlasEntry> AtlasGlyphEntries;
	robin_hood::unordered_map<char, size_t> CharMap;

	void Init(Graphics* t_Graphics);
	void InitNewAtlas();
	void Build(FontBuilder t_Builder);
	void LoadTypeface(MemoryArena t_Arena, float t_Size, size_t t_Index);
	AtlasEntry GetEntry(size_t t_TypeFace, char t_Ch);
	
};
