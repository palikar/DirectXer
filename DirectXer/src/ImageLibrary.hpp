#pragma once

#include <Glm.hpp>
#include <Memory.hpp>
#include <GraphicsCommon.hpp>
#include <Graphics.hpp>
#include <Platform.hpp>
#include <Assets.hpp>

#include <string_view>
#include <stb_rect_pack.h>

/*
  -> Loads images and puts them into textures
  -> One texture can contain several images and this
class keeps info on where exactly are those images - in which texture
and where exectly in this texture
  -> If the image that is to be loaded is "too big", we create a texure
for the whole image and do not put it into some shared texture
*/

struct Image
{
	TextureObject TexHandle;
	glm::vec2 ScreenPos;
	glm::vec2 ScreenSize;
	glm::vec2 AtlasSize;
};

struct ImageAtlas
{
	TextureObject TexHandle;
	stbrp_context RectContext;
};

struct ImageLibraryBuilder
{
public:

	struct QueuedImage
	{
		std::string_view Path;
		PlatformLayer::FileHandle Handle;
		size_t FileSize;
		uint32 Id;
	};

	struct MemoryImage
	{
		ImageHeader Image;
		void* Data;
	};

	TempVector<QueuedImage> QueuedImages;
	TempVector<MemoryImage> MemoryImages;
	size_t MaxFileSize;

	void Init(uint16 t_ImageCount);
	void PutImage(std::string_view t_Path, uint32 t_Id);
};

class ImageLibrary
{
  public:
	Graphics* Gfx;
	Map<uint32, Image> Images;
	BulkVector<ImageAtlas> Atlases;

	const static inline uint16 RectsCount = 1024u / 2u;
	const static inline uint16 AtlasSize = 1024u;
	const static inline uint8 MaxAtlases = 10u;

	void Init(Graphics* Gfx);
	ImageAtlas InitAtlas();
	TextureObject Pack(stbrp_rect& t_Rect);
	void Build(ImageLibraryBuilder& t_Builder);
	Image GetImage(uint32 t_id);

	void CreateMemoryImage(ImageHeader header, void* data)
	{
		assert(data);

		if (header.Width >= 1024 || header.Height >= 1024)
		{
			auto texture = Gfx->CreateTexture(header.Width, header.Height, header.Format, data, 0);
			Images.insert({ header.Id, Image{ texture, {0.0f, 0.0f}, {1.0f, 1.0f}, {header.Width, header.Height}}});
			return;
		}
		
		stbrp_rect rect;
		rect.w = (stbrp_coord)header.Width;
		rect.h = (stbrp_coord)header.Height;

		auto texture = Pack(rect);
		Gfx->UpdateTexture(texture, { {rect.x, rect.y}, { rect.w, rect.h }}, data);
		Images.insert({ header.Id, Image{ texture, {(float)rect.x / AtlasSize, (float)rect.y / AtlasSize}, {(float)rect.w / AtlasSize, (float)rect.h / AtlasSize}, {AtlasSize, AtlasSize} }});
	
	}
	
	void CreateStaticAtlas(AtlasEntry entry, void* data)
	{
		ImageAtlas newAtlas;
		newAtlas.TexHandle = Gfx->CreateTexture(entry.Width, entry.Height, entry.Format, data, 0);
		newAtlas.RectContext = {0};
		Atlases.push_back(newAtlas);
	}
	
};
