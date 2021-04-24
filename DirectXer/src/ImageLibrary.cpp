#pragma once

#include <Graphics.hpp>
#include <Resources.hpp>
#include <ImageLibrary.hpp>

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
	Assert(Atlases.size() <= MaxAtlases, "Too many image atlases. Try increasing the maximum atlases number");

	ImageAtlas newAtlas;
	newAtlas.TexHandle = NextTextureId();
	Gfx->CreateTexture(newAtlas.TexHandle, {ImageAtlasSize, ImageAtlasSize, TF_RGBA}, nullptr);
	Atlases.push_back(newAtlas);

	// @Note: 8Kb Per atlas for tect packing; maybe we can bump this to 16KB for best rect packing results
	auto space = Memory::BulkGet<stbrp_node>(RectsCount);
	stbrp_init_target(&Atlases.back().RectContext, ImageAtlasSize, ImageAtlasSize, space, RectsCount);

	return Atlases.back();
}

TextureId ImageLibrary::Pack(stbrp_rect& t_Rect)
{
	for (auto& atlas: Atlases)
	{
		if (atlas.RectContext.width == 0) continue;
		stbrp_pack_rects(&atlas.RectContext, &t_Rect, 1);
		if (t_Rect.was_packed != 0) return atlas.TexHandle;
	}
	auto newAtlas = InitAtlas();
	stbrp_pack_rects(&newAtlas.RectContext, &t_Rect, 1);

	Assert(t_Rect.was_packed != 0, "Can't pack image in a fresh atlas");
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

	Images.reserve(t_Builder.QueuedImages.size());
	
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
		
		CreateMemoryImage(queuedImage.Id, {(uint16)width, (uint16)height, TF_RGBA}, data);
	}
	
}

void ImageLibrary::CreateMemoryImage(ImageId id, ImageDescription desc, void* data)
{
	Assert(data, "This image does not have memory data");

	if (desc.Width >= MaxWidthForPacking|| desc.Height >= MaxHeightForPacking)
	{
		const auto texId = NextTextureId();
		Gfx->CreateTexture(texId, { (uint16)desc.Width, (uint16)desc.Height, desc.Format }, data);
		Images.insert({ ImageId{id}, Image{ texId, {0.0f, 0.0f}, {1.0f, 1.0f}, {desc.Width, desc.Height}} });
		return;
	}
		
	stbrp_rect rect;
	rect.w = (stbrp_coord)desc.Width;
	rect.h = (stbrp_coord)desc.Height;

	auto texture = Pack(rect);
	Gfx->UpdateTexture(texture, { {rect.x, rect.y}, { rect.w, rect.h }}, data);
	Images.insert({ ImageId{id}, Image{ texture, {(float)rect.x / ImageAtlasSize, (float)rect.y / ImageAtlasSize}, {(float)rect.w / ImageAtlasSize, (float)rect.h / ImageAtlasSize}, {ImageAtlasSize, ImageAtlasSize} }});
	
}
	
