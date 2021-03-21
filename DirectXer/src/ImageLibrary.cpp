#pragma once

#include "Graphics.hpp"
#include "PlatformWindows.hpp"
#include "Resources.hpp"
#include "ImageLibrary.hpp"

#include <stb_image.h>

void ImageLibraryBuilder::Init(uint16 t_ImageCount)
{
	QueuedImages.reserve(t_ImageCount);
	MaxFileSize = 0;
}

uint32 ImageLibraryBuilder::PutImage(std::string_view t_Path)
{
	auto path = Resources::ResolveFilePath(t_Path);
	QueuedImage newImage;
	newImage.Handle = PlatformLayer::OpenFileForReading(path);
	newImage.Path = t_Path;
	newImage.FileSize = PlatformLayer::FileSize(newImage.Handle);
	QueuedImages.push_back(newImage);

	MaxFileSize = MaxFileSize < newImage.FileSize ? newImage.FileSize  : MaxFileSize;

	return (uint32)QueuedImages.size() - 1;
}


void ImageLibrary::Init(Graphics* Gfx)
{
	this->Gfx = Gfx;
	InitAtlas();
	Atlases.reserve(MaxAtlases);
}

ImageAtlas ImageLibrary::InitAtlas()
{
	assert(Atlases.size() <= MaxAtlases);

	ImageAtlas newAtlas;
	newAtlas.TexHandle = Gfx->createTexture(AtlasSize, AtlasSize, TF_RGBA, nullptr, 0);
	Atlases.push_back(newAtlas);

	// @Note: 8Kb Per atlas for tect packing; maybe we can bump this to 16KB for best rect packing results
	auto space = Memory::BulkGet<stbrp_node>(RectsCount);
	stbrp_init_target(&Atlases.back().RectContext, AtlasSize, AtlasSize, space, RectsCount);

	return newAtlas;
}

TextureObject ImageLibrary::Pack(stbrp_rect& t_Rect)
{
	for (auto& atlas: Atlases)
	{
		stbrp_pack_rects(&atlas.RectContext, &t_Rect, 1);
		if (t_Rect.was_packed != 0) return atlas.TexHandle;
	}
	auto newAtlas = InitAtlas();
	stbrp_pack_rects(&newAtlas.RectContext, &t_Rect, 1);

	assert(t_Rect.was_packed != 0);
	return {};
}

void ImageLibrary::Build(ImageLibraryBuilder t_Builder)
{
	fileArena = Memory::GetTempArena(t_Builder.MaxFileSize + Megabytes(1));
	Memory::EstablishTempScope(Megabytes(32));
	Defer { 
		Memory::EndTempScope();
		Memory::DestoryTempArena(fileArena);
	};
	
	stbi_set_flip_vertically_on_load(0);

	for (auto& queuedImage : t_Builder.QueuedImages)
	{
		Defer {
			fileArena.Reset();
			Memory::ResetTempScope();
		};

		PlatformLayer::ReadFileIntoArena(queuedImage.Handle, queuedImage.FileSize, fileArena);

		int width, height, channels;
		unsigned char* data = stbi_load_from_memory((unsigned char*)fileArena.Memory, (int)fileArena.Size, &width, &height, &channels, 0);
 
		stbrp_rect rect;
		rect.w = (stbrp_coord)width;
		rect.h = (stbrp_coord)height;

		auto texture = Pack(rect);
		Gfx->updateTexture(texture, { {rect.x, rect.y}, { rect.w, rect.h }}, data);
		Images.push_back({ texture, {(float)rect.x / AtlasSize, (float)rect.y / AtlasSize}, {(float)rect.w / AtlasSize, (float)rect.h / AtlasSize} });


	}

}

Image ImageLibrary::GetImage(uint32 t_id)
{
	return Images[t_id];
}
