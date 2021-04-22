#include <Assets.hpp>
#include <FileUtils.hpp>

#include <ImageLibrary.hpp>
#include <Audio.hpp>
#include <FontLibrary.hpp>

void AssetStore::LoadAssetFile(String path, AssetBuildingContext& context)
{
	MemoryArena fileArena = Memory::GetTempArena(Megabytes(64));
	Defer { 
		Memory::DestoryTempArena(fileArena);
	};

	DXLOG("[Init] Loading asset file: {}", path);

	ReadWholeFile(path.data(), fileArena);
	auto current = fileArena.Memory;

	auto header = ReadBlob<AssetColletionHeader>(current);

	context.ImageLib->Images.reserve(header.ImagesCount);
	context.WavLib->AudioEntries.reserve(header.WavCount);
	// @Note: This *has* to be resize because we put straight into the array later
	context.FontLib->AtlasGlyphEntries.resize(header.FontsCount * FontLibrary::Characters.size());
	

	const auto assetsCount = header.ImagesCount + header.WavCount + header.FontsCount + header.AtlasesCount;

	for (uint32 i = 0; i < assetsCount; ++i)
	{
		auto assetEntry = ReadBlob<AssetEntry>(current);
		switch (assetEntry.Type)
		{
		  case Type_Image:
		  {
			  ImageHeader imageHeader = *((ImageHeader*)(fileArena.Memory + assetEntry.Offset));
			  void* data = (fileArena.Memory + assetEntry.Offset + sizeof(ImageHeader));
			  context.ImageLib->CreateMemoryImage(imageHeader, data);
			  break;
		  }

		  case Type_Atlas:
		  {
			  AtlasFileHeader* atlasHeader = ((AtlasFileHeader*)(fileArena.Memory + assetEntry.Offset));	

			  auto baseAtlasIndex = context.ImageLib->Atlases.size();
			  for (uint16 j = 0; j < atlasHeader->NumAtlases; ++j)
			  {
				  AtlasEntry* atlas = (AtlasEntry*)((char*)atlasHeader + sizeof(AtlasFileHeader) + j * sizeof(AtlasEntry));
				  context.ImageLib->CreateStaticAtlas(*atlas, ((char*)atlasHeader + atlas->Offset));
			  }

			  for (uint16 j = 0; j < atlasHeader->NumImages; ++j)
			  {
				  ImageEntry* image = (ImageEntry*)((char*)atlasHeader + sizeof(AtlasFileHeader) + j * sizeof(ImageEntry) + atlasHeader->NumAtlases * sizeof(AtlasEntry));

				  Image newImage;
				  newImage.ScreenPos = {image->X, image->Y};
				  newImage.ScreenSize = {image->Width, image->Height};
				  newImage.AtlasSize = {image->AtlasWidth, image->AtlasHeight};
				  newImage.TexHandle = context.ImageLib->Atlases[(size_t)(image->Atlas + baseAtlasIndex)].TexHandle;
				  context.ImageLib->Images.insert({image->Id, newImage });
			  }
			  
			  break;
		  }

		  case Type_Wav:
		  {
			  WavAssetHeader* wavHeader = ((WavAssetHeader*)(fileArena.Memory + assetEntry.Offset));
			  context.WavLib->CreateMemoryWav(*wavHeader, (char*)wavHeader + sizeof(WavAssetHeader));
			  break;
		  }

		  case Type_Font:
		  {
			  FontHeader* fontHeader = ((FontHeader*)(fileArena.Memory + assetEntry.Offset));			
			  context.FontLib->LoadTypeface(((char*)fontHeader + sizeof(FontHeader)), fontHeader->DataSize, (float)fontHeader->FontSize, fontHeader->Id);
			  break;
		  }
		  
		}
	}
}
