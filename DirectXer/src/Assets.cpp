#include <Assets.hpp>
#include <FileUtils.hpp>

#include <ImageLibrary.hpp>
#include <Graphics.hpp>
#include <Audio.hpp>
#include <FontLibrary.hpp>
#include <Timing.hpp>

// @Note: I hope this gets inlined; is is there because I am lazy at typing
template<typename T>
static void* GetData(MemoryArena& fileArena, T& entry)
{
	return fileArena.Memory + entry.DataOffset;
}

static void* GetData(MemoryArena& fileArena, size_t offset)
{
	return fileArena.Memory + offset;
}

void AssetStore::LoadAssetFile(AssetFile file, AssetBuildingContext& context)
{
	DxProfileCode(DxTimedBlock(Phase_Init, "Asset loading"));
	MemoryArena fileArena = Memory::GetTempArena(file.Size + Kilobytes(1));
	Defer { 
		Memory::DestoryTempArena(fileArena);
	};

	DXDEBUG("[Init] Loading asset file: {}", file.Path);

	ReadWholeFile(file.Path, fileArena);
	auto current = fileArena.Memory;

	auto header = ReadBlob<AssetColletionHeader>(current);

	context.ImageLib->Images.reserve(header.LoadImagesCount + header.ImagesCount);
	context.WavLib->AudioEntries.reserve(header.LoadWavsCount);
	context.FontLib->AtlasGlyphEntries.resize(header.LoadFontsCount * FontLibrary::Characters.size());
	
	for (uint32 i = 0; i < header.TexturesCount; ++i)
	{
		const TextureLoadEntry& entry = ReadBlob<TextureLoadEntry>(current);
		context.Graphics->CreateTexture(entry.Id, entry.Desc, GetData(fileArena, entry));
	}
	
	for (uint32 i = 0; i < header.ImagesCount; ++i)
	{
		const ImageEntry& entry = ReadBlob<ImageEntry>(current);
		context.ImageLib->Images.insert({ entry.Id, entry.Image });
	}

	for (uint32 i = 0; i < header.AtlasesCount; ++i)
	{
		const ImageAtlas& entry = ReadBlob<ImageAtlas>(current);
		context.ImageLib->Atlases.push_back(entry);
	}
	
	for (uint32 i = 0; i < header.LoadImagesCount; ++i)
	{
		const ImageLoadEntry& entry = ReadBlob<ImageLoadEntry>(current);
		context.ImageLib->CreateMemoryImage(entry.Id, entry.Desc, GetData(fileArena, entry));
	}	
	
	for (uint32 i = 0; i < header.LoadWavsCount; ++i)
	{
		const WavLoadEntry& entry = ReadBlob<WavLoadEntry>(current);
		context.WavLib->CreateMemoryWav(entry.Id, entry.Desc, GetData(fileArena, entry));
	}	

	for (uint32 i = 0; i < header.LoadFontsCount; ++i)
	{
		FontLoadEntry& entry = ReadBlob<FontLoadEntry>(current);
		context.FontLib->CreateMemoryTypeface(entry.Id, entry.Desc, GetData(fileArena, entry), entry.DataSize);
	}
		
	for (uint32 i = 0; i < header.SkyboxesCount; ++i)
	{
		SkyboxLoadEntry& entry = ReadBlob<SkyboxLoadEntry>(current);

		void* datas[] = {
			GetData(fileArena, entry.DataOffset[0]), 
			GetData(fileArena, entry.DataOffset[1]), 
			GetData(fileArena, entry.DataOffset[2]), 
			GetData(fileArena, entry.DataOffset[3]), 
			GetData(fileArena, entry.DataOffset[4]), 
			GetData(fileArena, entry.DataOffset[5]), 
		};
		context.Graphics->CreateCubeTexture(entry.Id, entry.Desc, datas);
	}

	for (uint32 i = 0; i < header.LoadMeshesCount; ++i)
	{
		MeshLoadEntry& entry = ReadBlob<MeshLoadEntry>(current);

		context.Graphics->CreateVertexBuffer(entry.Vbo, entry.VBDesc.StructSize,
											 GetData(fileArena, entry.DataOffsetVBO),
											 entry.VBDesc.DataSize);

		context.Graphics->CreateIndexBuffer(entry.Ibo, GetData(fileArena, entry.DataOffsetIBO),
			entry.VBDesc.DataSize);
		
	}

	
	
	
}
