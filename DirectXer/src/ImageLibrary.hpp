#pragma once

#include <Glm.hpp>
#include <Memory.hpp>
#include <Config.hpp>
#include <GraphicsCommon.hpp>
#include <Graphics.hpp>
#include <Platform.hpp>

#include <stb_rect_pack.h>

struct ImageDescription
{
	uint16 Width;
	uint16 Height;
	TextureFormat Format;
};

struct AtlasDescription
{
	int Width;
	int Height;
	TextureFormat Format;
};

using ImageId = uint32;

struct Image
{
	TextureId TexHandle;
	glm::vec2 ScreenPos;
	glm::vec2 ScreenSize;
	glm::vec2 AtlasSize;
};

struct ImageAtlas
{
	TextureId TexHandle;
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

	TempVector<QueuedImage> QueuedImages;
	size_t MaxFileSize;

	void Init(uint16 t_ImageCount);
	void PutImage(std::string_view t_Path, uint32 t_Id);
};

class ImageLibrary
{
  public:
	Graphics* Gfx;
	Map<ImageId, Image> Images;
	BulkVector<ImageAtlas> Atlases;

	void Init(Graphics* Gfx);
	TextureId Pack(stbrp_rect& t_Rect);
	void Build(ImageLibraryBuilder& t_Builder);
	
	ImageAtlas InitAtlas();
	void CreateMemoryImage(ImageId id, ImageDescription desc, void* data);
	
};
