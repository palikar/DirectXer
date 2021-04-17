#pragma once

#include "Graphics.hpp"
#include "Resources.hpp"
#include "ImageLibrary.hpp"

#include <stb_image.h>

void ImageLibraryBuilder::Init(uint16 t_ImageCount)
{
	QueuedImages.reserve(t_ImageCount);
	MaxFileSize = 0;
}

void ImageLibraryBuilder::PutImage(std::string_view t_Path, uint32 t_Id)
{
	auto path = Resources::ResolveFilePath(t_Path);
	QueuedImage newImage;
	newImage.Handle = PlatformLayer::OpenFileForReading(path);
	newImage.Path = t_Path;
	newImage.FileSize = PlatformLayer::FileSize(newImage.Handle);
	newImage.Id = t_Id;
	QueuedImages.push_back(newImage);

	MaxFileSize = MaxFileSize < newImage.FileSize ? newImage.FileSize  : MaxFileSize;
}


void ImageLibrary::Init(Graphics* Gfx)
{
	this->Gfx = Gfx;
	InitAtlas();
	Atlases.reserve(MaxAtlases);
	Images.reserve(64);
}

ImageAtlas ImageLibrary::InitAtlas()
{
	assert(Atlases.size() <= MaxAtlases);

	ImageAtlas newAtlas;
	newAtlas.TexHandle = Gfx->CreateTexture(AtlasSize, AtlasSize, TF_RGBA, nullptr, 0);
	Atlases.push_back(newAtlas);

	// @Note: 8Kb Per atlas for tect packing; maybe we can bump this to 16KB for best rect packing results
	auto space = Memory::BulkGet<stbrp_node>(RectsCount);
	stbrp_init_target(&Atlases.back().RectContext, AtlasSize, AtlasSize, space, RectsCount);

	return Atlases.back();
}

TextureObject ImageLibrary::Pack(stbrp_rect& t_Rect)
{
	for (auto& atlas: Atlases)
	{
		if (atlas.RectContext.width == 0) continue;
		stbrp_pack_rects(&atlas.RectContext, &t_Rect, 1);
		if (t_Rect.was_packed != 0) return atlas.TexHandle;
	}
	auto newAtlas = InitAtlas();
	stbrp_pack_rects(&newAtlas.RectContext, &t_Rect, 1);

	assert(t_Rect.was_packed != 0);
	return newAtlas.TexHandle;
}

void ImageLibrary::Build(ImageLibraryBuilder& t_Builder)
{
	MemoryArena fileArena = Memory::GetTempArena(t_Builder.MaxFileSize + Megabytes(1));
	Memory::EstablishTempScope(Megabytes(64));
	Defer { 
		Memory::EndTempScope();
		Memory::DestoryTempArena(fileArena);
	};

	Images.reserve(t_Builder.QueuedImages.size() + t_Builder.MemoryImages.size());
	
	stbi_set_flip_vertically_on_load(0);

	for (auto& queuedImage : t_Builder.QueuedImages)
	{
		Defer {
			fileArena.Reset();
			Memory::ResetTempScope();
		};

		PlatformLayer::ReadFileIntoArena(queuedImage.Handle, queuedImage.FileSize, fileArena);

		int width, height, channels;
		unsigned char* data = stbi_load_from_memory((unsigned char*)fileArena.Memory, (int)fileArena.Size, &width, &height, &channels, 4);

		assert(data);

		if (width >= 1024 || height >= 1024)
		{
			auto texture = Gfx->CreateTexture(width, height, TF_RGBA, data, 0);
			Images.insert({queuedImage.Id, Image{ texture, {0.0f, 0.0f}, {1.0f, 1.0f}, {width, height}}});
			continue;
		}
		
		stbrp_rect rect;
		rect.w = (stbrp_coord)width;
		rect.h = (stbrp_coord)height;

		auto texture = Pack(rect);
		Gfx->UpdateTexture(texture, { {rect.x, rect.y}, { rect.w, rect.h }}, data);
		Images.insert({queuedImage.Id, Image{ texture, {(float)rect.x / AtlasSize, (float)rect.y / AtlasSize}, {(float)rect.w / AtlasSize, (float)rect.h / AtlasSize}, {AtlasSize, AtlasSize} }});


	}

	for (auto& memoryImage : t_Builder.MemoryImages)
	{
		assert(memoryImage.Data);

		if (memoryImage.Image.Width >= 1024 || memoryImage.Image.Height >= 1024)
		{
			auto texture = Gfx->CreateTexture(memoryImage.Image.Width, memoryImage.Image.Height, memoryImage.Image.Format, memoryImage.Data, 0);
			Images.insert({ memoryImage.Image.Id, Image{ texture, {0.0f, 0.0f}, {1.0f, 1.0f}, {memoryImage.Image.Width, memoryImage.Image.Height}}});
			continue;
		}
		
		stbrp_rect rect;
		rect.w = (stbrp_coord)memoryImage.Image.Width;
		rect.h = (stbrp_coord)memoryImage.Image.Height;

		auto texture = Pack(rect);
		Gfx->UpdateTexture(texture, { {rect.x, rect.y}, { rect.w, rect.h }}, memoryImage.Data);
		Images.insert({ memoryImage.Image.Id, Image{ texture, {(float)rect.x / AtlasSize, (float)rect.y / AtlasSize}, {(float)rect.w / AtlasSize, (float)rect.h / AtlasSize}, {AtlasSize, AtlasSize} }});
	}
}

Image ImageLibrary::GetImage(uint32 t_id)
{
	return Images[t_id];
}
