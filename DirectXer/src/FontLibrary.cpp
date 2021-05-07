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
	auto texId = NextTextureId();
	Gfx->CreateTexture(texId, {AtlasSize, AtlasSize, TF_R}, nullptr);
	auto space = Memory::BulkGetType<stbrp_node>(RectsCount);
	stbrp_init_target(&RectContext, AtlasSize, AtlasSize, space,  RectsCount);
	Atlases.push_back(texId);
}

void FontLibrary::Build(FontBuilder t_Builder)
{
	MemoryArena fileArena = Memory::GetTempArena(t_Builder.MaxFileSize + Megabytes(1));
	Defer { 
		Memory::DestoryTempArena(fileArena);
	};

	AtlasGlyphEntries.reserve(t_Builder.LoadEntries.size() * Characters.size());
	AtlasGlyphEntries.resize(t_Builder.LoadEntries.size() * Characters.size());
		
	for (auto entry : t_Builder.LoadEntries)
	{
		PlatformLayer::ReadFileIntoArena(entry.Handle, entry.FileSize, fileArena);

		LoadTypeface(fileArena.Memory, fileArena.Size, entry.Size, IdMap.size());
		IdMap.insert({entry.Id, IdMap.size()});

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
	Assert(res == 0, "Can't open font face: {}", id);

	uint16 qcharIndex{0};
	for (const auto ch : Characters)
	{
		auto glyph_index = FT_Get_Char_Index( face, (unsigned long)ch );

		res = FT_Set_Pixel_Sizes(face, 0, FT_UInt(size));
		Assert(res == 0, "Can't set pixel size for typeface");
			
		res = FT_Load_Glyph(face, glyph_index, FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP);
		Assert(res == 0, "Can't load glyph: {}", glyph_index);
			
		res = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
		Assert(res == 0, "Can't render glyph: {}", glyph_index);

		const auto& bitmap = face->glyph->bitmap;
		const auto width = bitmap.width + Padding;
		const auto height = bitmap.rows + Padding;

		stbrp_rect rect;
		rect.w = (stbrp_coord)width;
		rect.h = (stbrp_coord)height;

		AtlasEntry entry;
		entry.TexHandle = 0;

		if (width != 0 && height != 0)
		{
			stbrp_pack_rects(&RectContext, &rect, 1);
			if(rect.was_packed == 0)
			{
				InitNewAtlas();
				stbrp_pack_rects(&RectContext, &rect, 1);
			}
			
			const auto glyphAtlas = Atlases.back();
			Gfx->UpdateTexture(glyphAtlas, { {rect.x + Padding, rect.y + Padding}, { bitmap.width, bitmap.rows}}, bitmap.buffer, 1);
		
			entry.Pos = glm::vec2{rect.x + Padding, rect.y + Padding} / (float)AtlasSize;
			entry.Size = glm::vec2{ bitmap.width, bitmap.rows} / (float)AtlasSize;
			entry.GlyphSize = glm::vec2{float(-face->glyph->bitmap_left), float(face->glyph->bitmap_top)};

			entry.TexHandle = glyphAtlas;
		}

		entry.Advance = glm::vec2(ToFloat(face->glyph->advance.x), ToFloat(face->glyph->advance.y));

		AtlasGlyphEntries[id*Characters.size() + qcharIndex++] = entry;
	}

	FT_Done_Face(face);		
}

void FontLibrary::CreateMemoryTypeface(FontId id, FontDescription desc, void* data, size_t size)
{
	LoadTypeface(data, size, desc.FontSize, IdMap.size());
	IdMap.insert({id, IdMap.size()});

}

FontLibrary::AtlasEntry FontLibrary::GetEntry(FontId typeFace, char ch)
{
	return AtlasGlyphEntries[IdMap.at(typeFace)*Characters.size() + CharMap[ch]];
}

void FontLibrary::GetEntries(FontId id, const char* text, size_t size, TempVector<AtlasEntry>& vec)
{
	size_t typeFace = IdMap.at(id) * Characters.size();
	for (size_t i = 0; i < size; ++i)
	{
		vec.push_back(AtlasGlyphEntries[typeFace + CharMap.at(text[i])]);
	}

}
