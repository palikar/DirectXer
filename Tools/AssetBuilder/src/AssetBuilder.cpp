#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>
#include <fstream>
#include <stb_image.h>
#include <GraphicsCommon.hpp>
#include <Types.hpp>
#include <Utils.hpp>
#include <Assets.hpp>
#include <fmt/format.h>
#include <filesystem>

#include <TexturePacker.hpp>
#include "AssetBuilder.hpp"


static AssetToLoad AssetsToLoad[] = {
	{Type_Font, Tag_Level, "fonts/DroidSans/DroidSans.ttf", "F_DroidSans", {24, 20}},
	{Type_Font, Tag_Level, "fonts/DroidSans/DroidSans-Bold.ttf", "F_DroidSansBold", {24, 20}},
	
	{Type_Wav, Tag_Level,  "shoot.wav", "A_SHOOT"},
	{Type_Wav, Tag_Level,  "explosion.wav", "A_EXPLODE"},
};

// @Note: The images here will be packed into atlases and those will be put in the asset bundle file.
static inline ImageToPack IagesForPacking[] = {

	{"assets/evil_ship_1.png", 64, 64, "I_EVIL_SHIP_1"},
	{"assets/evil_ship_2.png", 64, 64, "I_EVIL_SHIP_2"},
	{"assets/evil_ship_3.png", 64, 64, "I_EVIL_SHIP_3"},
	{"assets/PNG/Ship_Parts/Ship_Main_Icon.png", 64, 64, "I_MAIN_SHIP"},
	{"assets/ship_bullet.png", 32, 64, "I_BULLET"},
	{"assets/explosion.png", 0, 0, "I_EXPLOSION"},
	{"assets/heart.png", 32, 32, "I_HEART"},
	{"assets/PNG/Main_UI/Health_Bar_Table.png", 0, 0, "I_HEALTH"},
	{"assets/PNG/Main_Menu/BG.png", 0, 0, "I_BG"},
	{"assets/PNG/Main_UI/Stats_Bar.png", 0, 0, "I_STATS"},
};

static void ParseCommandLineArguments(int argc, char *argv[], AssetBuilder::CommandLineArguments& arguments)
{
	for (size_t i = 0; i < argc; ++i)
	{
		std::string current{argv[i]};
		if(current == "-r") {
			arguments.Root = argv[++i];
		} else if (current == "-o") {
			arguments.Output = argv[++i];
		} else if (current == "-m") {
			arguments.MaxSize = std::stoi(argv[++i]) * 1024 * 1024;
		} else if (current == "-h") {
			arguments.Header = argv[++i];
		} else if (current == "-i") {
			arguments.Id = argv[++i];
		}
	}
}

static size_t CalculateBaseOffset(AssetBundlerContext& context)
{
	size_t offset{0};

	offset += sizeof(AssetColletionHeader);
	offset += sizeof(TextureLoadEntry) * context.TexturesToCreate.size();
	offset += sizeof(ImageEntry) * context.Images.size();
	offset += sizeof(AtlasEntry) * context.Atlases.size();
	
	offset += sizeof(ImageLoadEntry) * context.LoadImages.size();
	offset += sizeof(WavLoadEntry) * context.LoadWavs.size();
	offset += sizeof(FontLoadEntry) * context.LoadFonts.size();

	return offset;
}

static void ApplyBaseOffset(AssetBundlerContext& context, size_t offset)
{
	for (auto& entry : context.TexturesToCreate) 
	{
		entry.DataOffset += offset; 
	}

	for (auto& entry : context.LoadImages) 
	{
		entry.DataOffset += offset; 
	}

	for (auto& entry : context.LoadWavs) 
	{
		entry.DataOffset += offset; 
	}

	for (auto& entry : context.LoadFonts) 
	{
		entry.DataOffset += offset; 
	}

}

int main(int argc, char *argv[])
{
	AssetBuilder::CommandLineArguments arguments{};
	ParseCommandLineArguments(argc, argv, arguments);

	AssetBundlerContext context;
	context.Header.VersionSpec = 1;

	AssetDataBlob dataBlob;
	dataBlob.Data.reserve(1024u*1024u*256u);

	TexturePacker::CommandLineArguments texturePackingArguments;
	texturePackingArguments.Root = arguments.Root;
	TexturePackerOutput packedImages = PackTextures(texturePackingArguments, IagesForPacking, size(IagesForPacking));
	
	// @Note: Create atlas entry and texture load entry for each atlas
	for (size_t i = 0; i < packedImages.Atlases.size(); ++i) 
	{
		auto& atlas = packedImages.Atlases[i];

		TextureLoadEntry texEntry;
		texEntry.Id = NextTextureAssetId();
		texEntry.Desc.Width = atlas.Width;
		texEntry.Desc.Height = atlas.Height;
		texEntry.Desc.Format = atlas.Format;
		texEntry.DataOffset = dataBlob.PutData(packedImages.AtlasesBytes[i]);
		context.TexturesToCreate.push_back(texEntry);
		
		NewAssetName(context, Type_Texture, fmt::format("T_ATLAS_{}", context.Atlases.size()).c_str(), texEntry.Id);

		std::cout << fmt::format("-- -> Bundling Texture [{:.3} MB]\n", (4 * texEntry.Desc.Width * texEntry.Desc.Height)/ 1024.0f*1024.0f);
		
		AtlasEntry atlEntry;
		atlEntry.Width = atlas.Width;  
		atlEntry.Height = atlas.Height;
		atlEntry.Format = atlas.Format;
		
		context.Atlases.push_back(atlEntry);
	}
	
	// @Note: Create image entry for each atlas image by using the tex handles generated
	// in the previous loop
	for (size_t i = 0; i < packedImages.Images.size(); ++i) 
	{
		auto atlasImage = packedImages.Images[i];
		ImageEntry imgEntry;
		imgEntry.Image.AtlasSize = { atlasImage.AtlasWidth, atlasImage.AtlasHeight};
		imgEntry.Image.ScreenPos = { atlasImage.X, atlasImage.Y};
		imgEntry.Image.ScreenSize = { atlasImage.Width, atlasImage.Height};
		imgEntry.Image.TexHandle = context.TexturesToCreate[atlasImage.Atlas].Id;
		
		imgEntry.Id = NewAssetName(context, Type_Image, atlasImage.Name);

		context.Images.push_back(imgEntry);
	}

	for (size_t i = 0; i < size(AssetsToLoad); ++i)
	{
		auto& asset = AssetsToLoad[i];
		asset.Path = fmt::format("{}/{}", arguments.Root, asset.Path);
		switch (asset.Type)
		{
		  case Type_Font: 
		  {
			  LoadFont(asset, context, dataBlob);
			  std::cout << fmt::format("{} -> Bundling font [{:.3} KB] [{}]\n", asset.Id, dataBlob.lastSize/1024.0f, asset.Path);
			  break;
		  }
		  case Type_Wav: 
		  {
			  LoadWav(asset, context, dataBlob);
			  std::cout << fmt::format("{} -> Bundling WAV [{:.3} KB] [{}]\n", asset.Id, dataBlob.lastSize/1024.0f, asset.Path);
			  break;
		  }
		  case Type_Image: 
		  {
			  LoadImage(asset, context, dataBlob);
			  std::cout << fmt::format("{} -> Bundling image [{:.3} MB] [{}]\n", asset.Id, dataBlob.lastSize/(1024.0f*1024.0f), asset.Path);
			  break;
		  }
		}
	}
	
	// @Note: The offsets in the context are relative to the beginning of the DataBlob;
	// when we put them on disk, some of the data in the context will be in front of the
	// pure data; hence we have to add this base offset to the offsets in the context
	size_t baseOffset = CalculateBaseOffset(context);
	ApplyBaseOffset(context, baseOffset);

	// @Note: From here forward, we are only writing the results of the packing to the files;
	// one header file and one asset bundle file
	
	std::string assetFileName = fmt::format("{}.dbundle", arguments.Output); 
	std::ofstream outfile(assetFileName, std::ios::out | std::ios::binary);

	/*
	  @Note: The output file will have the following format

	  |---------------Header-------------------| -- struct AssetColletionHeader
	  |----------------------------------------|
	  |---------------Texture_1----------------| -- struct TextureLoadEntry
	  |---------------Texture_2----------------|
	  |---------------.......------------------|
	  |---------------Texture_i----------------|
	  |----------------------------------------|
	  |---------------Image_1------------------| -- struct ImageEntry
	  |---------------Image_2------------------|
	  |---------------.......------------------|
	  |---------------Image_i------------------|
	  |----------------------------------------|
	  |---------------Atlas_1------------------| -- struct AtlasEntry
	  |---------------Atlas_2------------------|
	  |---------------.......------------------|
	  |---------------Atlas_i------------------|
	  |----------------------------------------|
	  |---------------Images-------------------| -- struct ImageLoadEntry
	  |----------------------------------------|
	  |----------------Wavs--------------------| -- struct WavLoadEntry
	  |----------------------------------------|
	  |---------------Fonts--------------------| -- struct FontsLoadEntry
	  |----------------------------------------|
	  |----------------DATA--------------------| -- unsigned char[]

	*/
	outfile.write((char*)&context.Header, sizeof(AssetColletionHeader));
	outfile.write((char*)&context.TexturesToCreate, sizeof(TextureLoadEntry)*context.TexturesToCreate.size());
	
	outfile.write((char*)&context.Images, sizeof(ImageEntry)*context.Images.size());
	outfile.write((char*)&context.Atlases, sizeof(AtlasEntry)*context.Atlases.size());

	outfile.write((char*)&context.LoadImages, sizeof(ImageLoadEntry)*context.LoadImages.size());
	outfile.write((char*)&context.LoadWavs, sizeof(WavLoadEntry)*context.LoadWavs.size());
	outfile.write((char*)&context.LoadFonts, sizeof(FontLoadEntry)*context.LoadFonts.size());

	outfile.write((char*)&context.Data, sizeof(char) * context.Data.size());
	
	outfile.close();

	std::ofstream headerFile(fmt::format("{}", arguments.Header), std::ios::out);
	headerFile << "#pragma once\n";

	headerFile << "\n\n";
	auto maxLength = std::max_element(context.Defines.begin(), context.Defines.end(), [](auto& d1, auto d2) { return d1.Name.size() < d2.Name.size(); })->Name.size();
	for(auto& define : context.Defines)
	{
		headerFile << fmt::format("#define {} {:>{}}", define.Name, define.Id, maxLength + 4 - define.Name.size()) << "\n";
	}
	headerFile << "\n\n";
	
	// headerFile << "static inline const char* ImageAssets[] = {\n";
	// for(auto& asset : AssetsToLoad)
	// 	if(asset.Type == Type_Image) headerFile << fmt::format("\t\"{}\",\n", asset.Id);
	// headerFile << "};\n\n";

	// headerFile << "static inline const char* FontAssets[] = {\n";
	// for(auto& asset : AssetsToLoad)
	// 	if(asset.Type == Type_Font) headerFile << fmt::format("\t\"{}\",\n", asset.Id);
	// headerFile << "};\n\n";

	// headerFile << "static inline const char* WavAssets[] = {\n";
	// for(auto& asset : AssetsToLoad)
	// 	if(asset.Type == Type_Wav) headerFile << fmt::format("\t\"{}\",\n", asset.Id);
	// headerFile << "};\n\n";

	headerFile << fmt::format("static inline const char* {}AssetFilePath = \"{}\";", arguments.Id, assetFileName);
	headerFile << "\n\n";
	
    return 0;
}

// @Todo: Materials Loading
// @Todo: Objects Loading
