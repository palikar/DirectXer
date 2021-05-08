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

/*
  @Note: Checklist for adding as new asset type

  1. Add the new type to the AssetType enum
  2. Create a new load structure that will be read by the loading code (in Assets.hpp)
  3. Adjust the base offset calculation (in Calculatebaseoffset)				 
  4. Apply the base offset to the entries of the new structure (in Applybaseoffset)
  5. Create new counter variable and its handling (in AssetColletionHeader in Assets.hpp)
  6. Create a new loader
  7. Output the loaded data to the final asset file

*/

static AssetToLoad AssetsToLoad[] = {

	{Type_Mesh, Tag_Level,  "models/first_tree.obj", "M_TREE_1"},
	
	{Type_Skybox, Tag_Level, "sky", "T_SKY"},

	{Type_Texture, Tag_Level, "checker.png", "T_CHECKER"},
	{Type_Texture, Tag_Level, "rocks_color.png", "T_ROCKS_COLOR"},
	{Type_Texture, Tag_Level, "floor_color.png", "T_FLOOR_COLOR"},
	{Type_Texture, Tag_Level, "rocks_ao.png", "T_ROCKS_AO"},
	{Type_Texture, Tag_Level, "rocks_normal.png", "T_ROCKS_NORMAL"},	

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
	{"assets/sprites.png", 0, 0, "I_SHOOT"},
	{"images/facebook.png", 0, 0, "I_FACEBOOK"},
	{"images/instagram.png", 0, 0, "I_INSTAGRAM"},
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
	offset += sizeof(IBLoadEntry) * context.IBsToCreate.size();
	offset += sizeof(VBLoadEntry) * context.VBsToCreate.size();

	offset += sizeof(ImageEntry) * context.Images.size();
	offset += sizeof(ImageAtlas) * context.Atlases.size();

	offset += sizeof(MaterialLoadEntry) * context.Materials.size();
	
	offset += sizeof(SkyboxLoadEntry) * context.Skyboxes.size();
	offset += sizeof(ImageLoadEntry) * context.LoadImages.size();
	offset += sizeof(WavLoadEntry) * context.LoadWavs.size();
	offset += sizeof(FontLoadEntry) * context.LoadFonts.size();
	offset += sizeof(MeshLoadEntry) * context.LoadMeshes.size();

	return offset;
}

static void ApplyBaseOffset(AssetBundlerContext& context, size_t offset)
{
	for (auto& entry : context.TexturesToCreate) 
	{
		entry.DataOffset += offset; 
	}

	for (auto& entry : context.VBsToCreate) 
	{
		entry.DataOffset += offset; 
	}

	for (auto& entry : context.IBsToCreate)
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

	for (auto& entry : context.Skyboxes) 
	{
		entry.DataOffset[0] += offset; 
		entry.DataOffset[1] += offset; 
		entry.DataOffset[2] += offset; 
		entry.DataOffset[3] += offset; 
		entry.DataOffset[4] += offset; 
		entry.DataOffset[5] += offset; 
	}

}

static void GenerateHeaderArrays(std::ofstream& headerFile, AssetBundlerContext& context, AssetType type, std::string_view arrayName)
{
	headerFile << fmt::format("static inline const char* {}Names[] = {{\n", arrayName);
	for(auto& define : context.Defines)
		if(define.Type == type) headerFile << fmt::format("\t\"{}\",\n", define.Name);
	headerFile << "};\n\n";
	headerFile << fmt::format("static inline uint32 {}Ids[] = {{\n", arrayName);
	for(auto& define : context.Defines)
		if(define.Type == type) headerFile << fmt::format("\t{},\n", define.Id);
	headerFile << "};\n\n";
	headerFile << "/* ------------------------------  */\n\n";
}

static void GenerateHeaderDefines(std::ofstream& headerFile, AssetBundlerContext& context)
{
	auto maxLength = std::max_element(context.Defines.begin(), context.Defines.end(), [](auto& d1, auto d2) { return d1.Name.size() < d2.Name.size(); })->Name.size();
	for(auto& define : context.Defines)
	{
		headerFile << fmt::format("#define {} {:>{}}", define.Name, define.Id, maxLength + 4 - define.Name.size()) << "\n";
	}
	headerFile << "\n\n";
}

int main(int argc, char *argv[])
{
	AssetBuilder::CommandLineArguments arguments{};
	ParseCommandLineArguments(argc, argv, arguments);

	AssetBundlerContext context;
	context.Header.VersionSpec = 1;

	AssetDataBlob dataBlob;
	dataBlob.Data.reserve(1024u*1024u*256u);
	
	fmt::print("----------Running the texture packer----------\n");
	TexturePacker::CommandLineArguments texturePackingArguments;
	texturePackingArguments.Root = arguments.Root;
	TexturePackerOutput packedImages = PackTextures(texturePackingArguments, IagesForPacking, size(IagesForPacking));
	fmt::print("----------Done with image packing----------\n");
	
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
		
		auto texName = fmt::format("T_ATLAS_{}", context.Atlases.size());
		NewAssetName(context, Type_Texture, texName.c_str(), texEntry.Id);

		std::cout << fmt::format("{} \t->\t Bundling Texture [{:.3} MB]\n", texName, dataBlob.lastSize / (1024.0f*1024.0f));
		
		ImageAtlas atlEntry;
		atlEntry.TexHandle = texEntry.Id;
		
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
			  std::cout << fmt::format("{} \t->\t Bundling Font [{:.3} KB] [{}]\n", asset.Id, dataBlob.lastSize/1024.0f, asset.Path);
			  break;
		  }
		  case Type_Wav: 
		  {
			  LoadWav(asset, context, dataBlob);
			  std::cout << fmt::format("{} \t->\t Bundling WAV [{:.3} KB] [{}]\n", asset.Id, dataBlob.lastSize/1024.0f, asset.Path);
			  break;
		  }
		  case Type_Image: 
		  {
			  LoadImage(asset, context, dataBlob);
			  std::cout << fmt::format("{} \t->\t Bundling Image [{:.3} MB] [{}]\n", asset.Id, dataBlob.lastSize/(1024.0f*1024.0f), asset.Path);
			  break;
		  }
		  case Type_Texture: 
		  {
			  LoadTexture(asset, context, dataBlob);
			  std::cout << fmt::format("{} \t->\t Bundling Texture [{:.3} MB] [{}]\n", asset.Id, dataBlob.lastSize/(1024.0f*1024.0f), asset.Path);
			  break;
		  }
		  case Type_Skybox: 
		  {
			  LoadSkybox(asset, context, dataBlob);
			  std::cout << fmt::format("{} \t->\t Bundling Skybox [{:.3} MB] [{}]\n", asset.Id, (4*dataBlob.lastSize)/(1024.0f*1024.0f), asset.Path);
			  break;
		  }
		  case Type_Mesh: 
		  {
			  LoadMesh(asset, context, dataBlob);
			  const float mem = dataBlob.lastSize > 1024*1024 ? dataBlob.lastSize / (1024.0f*1024.0f) : dataBlob.lastSize / 1024.0f;
			  const char* unit = dataBlob.lastSize > 1024*1024 ? "MBs" : "KBs";
			  std::cout << fmt::format("{} \t->\t Bundling Mesh [{:.3} {}] [{}]\n", asset.Id, mem, unit, asset.Path);
			  break;
		  }
		}
	}

	context.Header.TexturesCount = (uint32)context.TexturesToCreate.size();
	context.Header.VBsCount = (uint32)context.VBsToCreate.size();
	context.Header.IBsCount = (uint32)context.IBsToCreate.size();
	context.Header.ImagesCount = (uint32)context.Images.size();
	context.Header.AtlasesCount = (uint32)context.Atlases.size();
	context.Header.LoadImagesCount = (uint32)context.LoadImages.size();
 	context.Header.LoadWavsCount = (uint32)context.LoadWavs.size();
 	context.Header.LoadFontsCount  = (uint32)context.LoadFonts.size();
	context.Header.SkyboxesCount  = (uint32)context.Skyboxes.size();
	context.Header.LoadMeshesCount  = (uint32)context.LoadMeshes.size();
	context.Header.MaterialsCount  = (uint32)context.Materials.size();
	
	fmt::print("----------Done building assets----------\n");
	fmt::print("Textures: \t[{}]\n", context.Header.TexturesCount);
	fmt::print("Images: \t[{}]\n", context.Header.ImagesCount);
	fmt::print("Atlases: \t[{}]\n", context.Header.AtlasesCount);
	fmt::print("LoadImages: \t[{}]\n", context.Header.LoadImagesCount);
	fmt::print("LoadWavs: \t[{}]\n", context.Header.LoadWavsCount);
	fmt::print("LoadFonts: \t[{}]\n", context.Header.LoadFontsCount);
	fmt::print("Skyboxes: \t[{}]\n", context.Header.SkyboxesCount);
	fmt::print("Meshes: \t[{}]\n", context.Header.LoadMeshesCount);
	fmt::print("Materials: \t[{}]\n", context.Header.MaterialsCount);
	
	// @Note: The offsets in the context are relative to the beginning of the DataBlob;
	// when we put them on disk, some of the data in the context will be in front of the
	// pure data; hence we have to add this base offset to the offsets in the context
	size_t baseOffset = CalculateBaseOffset(context);
	ApplyBaseOffset(context, baseOffset);

	// @Note: From here forward, we are only writing the results of the packing to the files;
	// one header file and one asset bundle file
	
	/*
	  @Note: The output file will have the following format

	  |---------------Header-------------------| -- struct AssetColletionHeader
	  |----------------------------------------|
	  |---------------Texture_1----------------| -- struct TextureLoadEntry
	  |---------------Texture_2----------------|
	  |---------------.......------------------|
	  |---------------Texture_i----------------|
	  |----------------------------------------|
	  |------------------VB_i------------------| -- struct VBLoadEntry
	  |----------------------------------------|
	  |------------------IB_i------------------| -- struct IBLoadEntry
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
	  |---------------Skyboxes-----------------| -- struct SkyboxLoadEntry
	  |----------------------------------------|
	  -----------------Meshes------------------| -- struct MeshLoadEntry
	  |----------------------------------------|
	  |---------------Material-----------------| -- struct MaterialLoadEntry
	  |----------------------------------------|
	  |----------------DATA--------------------| -- unsigned char[]

	*/
	
	std::string assetFileName = fmt::format("{}.dbundle", arguments.Output);

	fmt::print("Saving [{} Bytes] of meta data in [{}]\n", baseOffset, assetFileName);
	fmt::print("Saving [{:.3} MBs] of pure data in [{}]\n", dataBlob.Data.size() / (1024.0f* 1024.0f), assetFileName);

	std::ofstream outfile(assetFileName, std::ios::out | std::ios::binary);

	outfile.write((char*)&context.Header, sizeof(AssetColletionHeader));
	
	outfile.write((char*)context.TexturesToCreate.data(), sizeof(TextureLoadEntry)*context.TexturesToCreate.size());
	outfile.write((char*)context.VBsToCreate.data(), sizeof(VBLoadEntry)*context.VBsToCreate.size());
	outfile.write((char*)context.IBsToCreate.data(), sizeof(IBLoadEntry)*context.IBsToCreate.size());
	
	outfile.write((char*)context.Images.data(), sizeof(ImageEntry)*context.Images.size());
	outfile.write((char*)context.Atlases.data(), sizeof(ImageAtlas)*context.Atlases.size());

	outfile.write((char*)context.LoadImages.data(), sizeof(ImageLoadEntry)*context.LoadImages.size());
	outfile.write((char*)context.LoadWavs.data(), sizeof(WavLoadEntry)*context.LoadWavs.size());
	outfile.write((char*)context.LoadFonts.data(), sizeof(FontLoadEntry)*context.LoadFonts.size());
	outfile.write((char*)context.Skyboxes.data(), sizeof(SkyboxLoadEntry)*context.Skyboxes.size());
	outfile.write((char*)context.LoadMeshes.data(), sizeof(MeshLoadEntry)*context.LoadMeshes.size());
	outfile.write((char*)context.Materials.data(), sizeof(MaterialLoadEntry)*context.Materials.size());

	outfile.write((char*)dataBlob.Data.data(), sizeof(char) * dataBlob.Data.size());
	
	outfile.close();

	std::ofstream headerFile(fmt::format("{}", arguments.Header), std::ios::out);
	headerFile << "#pragma once\n";
	headerFile << "\n\n";

	// @Note: Generate #define with the id of every asset
	GenerateHeaderDefines(headerFile, context);
	
	// @Note: Generaet arrays with the names and ids of each asset
	GenerateHeaderArrays(headerFile, context, Type_Image, "Images");
	GenerateHeaderArrays(headerFile, context, Type_Wav, "Wavs");
	GenerateHeaderArrays(headerFile, context, Type_Font, "Fonts");
	GenerateHeaderArrays(headerFile, context, Type_Mesh, "Meshes");
	
	headerFile << fmt::format("static inline AssetFile AssetFiles[] = {{\n");
	headerFile << fmt::format("\t{{ \"{}\", {} }},\n", assetFileName, baseOffset + dataBlob.Data.size());
	headerFile << fmt::format("}};\n\n");

	headerFile << fmt::format("static inline size_t {} = {};\n", arguments.Id , 0);
	
    return 0;
}

// @Todo: Materials Loading
// @Todo: Objects Loading
