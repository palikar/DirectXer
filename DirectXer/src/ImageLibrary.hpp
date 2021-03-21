#pragma once

#include "Glm.hpp"
#include "Memory.hpp"
#include "GraphicsCommon.hpp"
#include "PlatformWindows.hpp"

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

class ImageLibraryBuilder
{
public:

	struct QueuedImage
	{
		std::string_view Path;
		PlatformLayer::FileHandle Handle;
		size_t FileSize;
	};

	TempVector<QueuedImage> QueuedImages;
	size_t MaxFileSize;

	void Init(uint16 t_ImageCount);
	uint32 PutImage(std::string_view t_Path);

};

class Graphics;

class ImageLibrary
{
  public:
	Graphics* Gfx;
	MemoryArena fileArena;
	BulkVector<Image> Images;
	BulkVector<ImageAtlas> Atlases;

	const static inline uint16 RectsCount = 1024u / 2u;
	const static inline uint16 AtlasSize = 1024u;
	const static inline uint8 MaxAtlases = 10u;

	void Init(Graphics* Gfx);
	ImageAtlas InitAtlas();
	TextureObject Pack(stbrp_rect& t_Rect);
	void Build(ImageLibraryBuilder& t_Builder);
	Image GetImage(uint32 t_id);
	
};
