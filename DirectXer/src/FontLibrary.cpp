#include "FontLibrary.hpp"
#include "Resources.hpp"

void FontBuilder::Init(size_t t_Size)
{
	LoadEntries.reserve(t_Size);
	MaxFileSize = 0;
}

size_t FontBuilder::PutTypeface(std::string_view t_Path, float t_Size)
{
	auto path = Resources::ResolveFilePath(t_Path);
	FontLoadEntry newEntry;
	newEntry.Path = t_Path;
	newEntry.Handle = PlatformLayer::OpenFileForReading(path);
	newEntry.FileSize = PlatformLayer::FileSize(newEntry.Handle);
	newEntry.Size = t_Size;
	LoadEntries.push_back(newEntry);

	MaxFileSize = MaxFileSize < newEntry.FileSize ? newEntry.FileSize  : MaxFileSize;

	return LoadEntries.size() - 1;
};

void FontLibrary::Init(Graphics* t_Graphics)
{
	FT_Init_FreeType(&FTLibrary);
	Gfx = t_Graphics;

	Atlases.reserve(3);
	
	InitNewAtlas();
	CharMap.reserve(Characters.size());

	size_t charIndex{0};
	for (auto ch : Characters)
	{
		CharMap.insert({ch, charIndex++});
	}
}
	
void FontLibrary::InitNewAtlas()
{
	auto tex = Gfx->CreateTexture(AtlasSize, AtlasSize, TF_R, nullptr, 0);
	auto space = Memory::BulkGet<stbrp_node>(RectsCount);
	stbrp_init_target(&RectContext, AtlasSize, AtlasSize, space,  RectsCount);
	Atlases.push_back(tex);			
}

void FontLibrary::Build(FontBuilder t_Builder)
{
	MemoryArena fileArena = Memory::GetTempArena(t_Builder.MaxFileSize + Megabytes(1));
	Defer { 
		Memory::DestoryTempArena(fileArena);
	};

	AtlasGlyphEntries.reserve(t_Builder.LoadEntries.size() * Characters.size());
	AtlasGlyphEntries.resize(t_Builder.LoadEntries.size() * Characters.size());
		
	size_t faceIndex{ 0 };
	for (auto entry : t_Builder.LoadEntries)
	{
		PlatformLayer::ReadFileIntoArena(entry.Handle, entry.FileSize, fileArena);
		LoadTypeface(fileArena.Memory, fileArena.Size, entry.Size, faceIndex++);
		fileArena.Reset();
	}		
}
	
void FontLibrary::LoadTypeface(void* data, size_t dataSize, float size, size_t id)
{
	FT_Face face;

	FT_Open_Args openArgs;
	openArgs.flags = FT_OPEN_MEMORY;
	openArgs.memory_base = (uint8*)data;
	openArgs.memory_size = (FT_Long)dataSize;
		
	FT_Error res;
	res = FT_Open_Face(FTLibrary, &openArgs, 0, &face);
	assert(res == 0);

	uint16 qcharIndex{0};
	for (const auto ch : Characters)
	{
		auto glyph_index = FT_Get_Char_Index( face, (unsigned long)ch );

		res = FT_Set_Pixel_Sizes(face, 0, FT_UInt(size));
		assert(res == 0);
			
		res = FT_Load_Glyph(face, glyph_index, FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP);
		assert(res == 0);
			
		res = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
		assert(res == 0);

		const auto& bitmap = face->glyph->bitmap;
		const auto width = bitmap.width + Padding;
		const auto height = bitmap.rows + Padding;

		stbrp_rect rect;
		rect.w = (stbrp_coord)width;
		rect.h = (stbrp_coord)height;
		stbrp_pack_rects(&RectContext, &rect, 1);

		if(rect.was_packed == 0)
		{
			InitNewAtlas();
			stbrp_pack_rects(&RectContext, &rect, 1);
		}

		auto glyphAtlas = Atlases.back();
		Gfx->UpdateTexture(glyphAtlas, { {rect.x + Padding, rect.y + Padding}, { bitmap.width, bitmap.rows}}, bitmap.buffer, 1);

		AtlasEntry entry;
		entry.Pos = glm::vec2{rect.x + Padding, rect.y + Padding} / (float)AtlasSize;
		entry.Size = glm::vec2{ bitmap.width, bitmap.rows} / (float)AtlasSize;
		entry.GlyphSize = glm::vec2{float(-face->glyph->bitmap_left), float(face->glyph->bitmap_top)};

		entry.Advance = glm::vec2(ToFloat(face->glyph->advance.x), ToFloat(face->glyph->advance.y));
			
		entry.TexHandle = glyphAtlas;

		AtlasGlyphEntries[id*Characters.size() + qcharIndex++] = entry;
	}

	FT_Done_Face(face);		
}

FontLibrary::AtlasEntry FontLibrary::GetEntry(size_t typeFace, char ch)
{
	return AtlasGlyphEntries[typeFace*Characters.size() + CharMap[ch]];
}
