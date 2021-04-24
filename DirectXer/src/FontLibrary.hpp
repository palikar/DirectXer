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

using FontId = uint32;

struct FontDescription
{
	float FontSize;
};

struct FontBuilder
{
	struct FontLoadEntry
	{
		std::string_view Path;
		float Size;
		PlatformLayer::FileHandle Handle;
		size_t FileSize;
		FontId Id;
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
		" "
		"°!\"§$%&/(){}[]=?`´'><|,.-;:_#+*~öäüÖÄÜ";

	struct AtlasEntry
	{
		glm::vec2 Pos;
		glm::vec2 Size;
		TextureId TexHandle;
		glm::vec2 GlyphSize;
		glm::vec2 Advance;		
	};

	FT_Library FTLibrary;
	Graphics* Gfx;
	BulkVector<TextureId> Atlases;
	stbrp_context RectContext;
	BulkVector<AtlasEntry> AtlasGlyphEntries;
	Map<char, size_t> CharMap;
	Map<FontId, size_t> IdMap;

	void Init(Graphics* t_Graphics);
	void InitNewAtlas();
	void Build(FontBuilder t_Builder);
	void LoadTypeface(void* data, size_t dataSize, float size, size_t id);

	void CreateMemoryTypeface(FontId id, FontDescription desc, void* data, size_t size);
	
	AtlasEntry GetEntry(FontId typeFace, char ch);
	void GetEntries(FontId id, const char* text, size_t size, TempVector<AtlasEntry>& vec);
};
