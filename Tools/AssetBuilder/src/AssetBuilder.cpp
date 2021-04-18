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

#include "AssetBuilder.hpp"

static AssetToLoad AssetsToLoad[] = {

	{Type_Image, Tag_Level, "assets/PNG/Main_Menu/BG.png", "I_BG"},
	{Type_Image, Tag_Level, "assets/PNG/Main_UI/Stats_Bar.png", "I_STATS"},

	{Type_Font, Tag_Level, "fonts/DroidSans/DroidSans.ttf", "F_DroidSans", {24, 20}},
	{Type_Font, Tag_Level, "fonts/DroidSans/DroidSans-Bold.ttf", "F_DroidSansBold", {24, 20}},
	
	{Type_Wav, Tag_Level,  "shoot.wav", "A_SHOOT"},
	{Type_Wav, Tag_Level,  "explosion.wav", "A_EXPLODE"},

	// @Note: The image IDs are in the atlas itself
	{Type_Atlas, Tag_Level, "SpaceGameAtlases.datlas", "<not used>"},
};

struct CommandLineArguments
{
	std::string Root{""};
	std::string Output{"output"}; 
	std::string Header{"output"};
	std::string Id{"Asset"};
	size_t MaxSize{128};
};

void ParseCommandLineArguments(int argc, char *argv[], CommandLineArguments& arguments)
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

int main(int argc, char *argv[])
{
	CommandLineArguments arguments{};
	ParseCommandLineArguments(argc, argv, arguments);

	AssetColletionHeader header{ 0 };
	header.VersionSpec = 1;

	std::vector<AssetEntry> entries;
	entries.reserve(size(AssetsToLoad));
	
	std::vector<unsigned char> data;
	data.reserve(Megabytes(arguments.MaxSize));

	std::vector<std::string> headerDefines;
	
	size_t currentOffset = size(AssetsToLoad) * sizeof(AssetEntry) + sizeof(AssetColletionHeader);
	
	for (size_t i = 0; i < size(AssetsToLoad); ++i)
	{
		auto& asset = AssetsToLoad[i];
		asset.Path = fmt::format("{}/{}", arguments.Root, asset.Path);
		size_t size = 0;

		switch (asset.Type)
		{
		  case Type_Image: 
		  {
			  headerDefines.push_back(fmt::format("#define {}\t{}", asset.Id, header.ImagesCount));
			  size = LoadImage(asset, data, header.ImagesCount);
			  header.ImagesCount += 1;
			  std::cout << fmt::format("{} -> Packing image [{:.3} {}] [{}]\n", asset.Id, size/(1024.0f*1024.0f), size > 1024*1024 ? "MB" : "KB", asset.Path);
			  break;
		  }
		  case Type_Font: 
		  {
			  headerDefines.push_back(fmt::format("#define {}_{} \t {}", asset.Id, asset.data.unsigned1, header.FontsCount));
			  size = LoadFont(asset, data, header.FontsCount);
			  header.FontsCount += 1;
			  std::cout << fmt::format("{} -> Packing font [{:.3} KB] [{}]\n", asset.Id, size/1024.0f, asset.Path);
			  break;
		  }
		  case Type_Wav: 
		  {
			  headerDefines.push_back(fmt::format("#define {}\t{}", asset.Id, header.WavCount));
			  header.WavCount += 1;
			  size = LoadWav(asset, data);
			  std::cout << fmt::format("{} -> Packing WAV [{:.3} KB] [{}]\n", asset.Id, size/1024.0f, asset.Path);
			  break;
		  }
		  case Type_Atlas: 
		  {
			  header.AtlasesCount += 1;
			  size = LoadAtlas(asset, data, headerDefines, header);
			  std::cout << fmt::format("----- -> Packing Atlas [{:.3} KB] [{}]\n", size/1024.0f, asset.Path);
			  break;
		  }
		}
		
		AssetEntry entry{ 0 };
		entry.Offset = currentOffset;
		entry.Size = size;
		entry.Type = asset.Type;
		entry.Tag = asset.TagField;
		entries.push_back(entry);

		currentOffset += size;
	}

	auto size = sizeof(AssetColletionHeader) + entries.size() * sizeof(AssetEntry) + data.size();
	std::cout << fmt::format("Creating file [{:.3} MB] [{}.dx1]\n",size/(1024.0f*1024.0f), arguments.Output);

	std::string assetFileName = fmt::format("{}.dbundle", arguments.Output); 
	std::ofstream outfile(assetFileName, std::ios::out | std::ios::binary);

	outfile.write((char*)&header, sizeof(AssetColletionHeader));
	outfile.write((char*)entries.data(), entries.size() * sizeof(AssetEntry));
	outfile.write((char*)data.data(), data.size());

	outfile.flush();
	outfile.close();

	std::cout << fmt::format("Creating file [{}.hpp] with [{}] defines\n", arguments.Header, headerDefines.size());
		
	std::ofstream headerFile(fmt::format("{}.hpp", arguments.Header), std::ios::out);
	headerFile << "#pragma once\n";

	headerFile << "\n\n";
	for(auto& define : headerDefines)
	{
		headerFile << define << "\n";
	}
	headerFile << "\n\n";
	
	headerFile << "static inline const char* ImageAssets[] = {\n";
	for(auto& asset : AssetsToLoad)
		if(asset.Type == Type_Image) headerFile << fmt::format("\t\"{}\",\n", asset.Id);
	headerFile << "};\n\n";

	headerFile << "static inline const char* FontAssets[] = {\n";
	for(auto& asset : AssetsToLoad)
		if(asset.Type == Type_Font) headerFile << fmt::format("\t\"{}\",\n", asset.Id);
	headerFile << "};\n\n";

	headerFile << "static inline const char* WavAssets[] = {\n";
	for(auto& asset : AssetsToLoad)
		if(asset.Type == Type_Wav) headerFile << fmt::format("\t\"{}\",\n", asset.Id);
	headerFile << "};\n\n";

	headerFile << fmt::format("static inline const char* {}AssetFilePath = \"{}\";", arguments.Id, assetFileName);
	headerFile << "\n\n";
	
    return 0;
}

// @Todo: Texture Loading
// @Todo: Materials Loading
// @Todo: Objects Loading

// @Todo: Asset bundle description file
