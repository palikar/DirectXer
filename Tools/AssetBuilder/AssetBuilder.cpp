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
	{Type_Image, Tag_Level, "assets/PNG/Ship_Parts/Ship_Main_Icon.png", "I_SHIP"},
	{Type_Image, Tag_Level, "assets/evil_ship_1.png", "I_ENEMY_1"},
	{Type_Image, Tag_Level, "assets/evil_ship_2.png", "I_ENEMY_2"},
	{Type_Image, Tag_Level, "assets/evil_ship_3.png", "I_ENEMY_3"},
	{Type_Image, Tag_Level, "assets/ship_bullet.png", "i_BULLET"},
	{Type_Image, Tag_Level, "assets/explosion.png", "i_EXPLODE"},
	{Type_Image, Tag_Level, "assets/PNG/Main_UI/Stats_Bar.png", "I_STATS"},
	{Type_Image, Tag_Level, "assets/PNG/Main_UI/Health_Bar_Table.png", "i_HEALTH"},
	{Type_Image, Tag_Level, "assets/heart.png", "I_HEART"},

	{Type_Font, Tag_Level, "fonts/DroidSans/DroidSans.ttf", "F_DroidSans", {24, 20}},
	{Type_Font, Tag_Level, "fonts/DroidSans/DroidSans-Bold.ttf", "F_DroidSansBold", {24, 20}},
	
	{Type_Wav, Tag_Level,  "shoot.wav", "A_SHOOT"},
	{Type_Wav, Tag_Level,  "explosion.wav", "A_EXPLODE"},
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
		
		if(asset.Type == Type_Image)
		{
			headerDefines.push_back(fmt::format("#define {}\t{}", asset.Id, header.ImagesCount));
			header.ImagesCount += 1;
			size = LoadImage(asset, data);
			if(size > 1024*1024)
			{
				std::cout << fmt::format("{} -> Packing image [{:.3} MB] [{}]\n", asset.Id, size/(1024.0f*1024.0f), asset.Path); 
			}
			else
			{
				std::cout << fmt::format("{} -> Packing image [{:.3} KB] [{}]\n", asset.Id, size/1024.0f, asset.Path); 
			}
		}
		else if(asset.Type == Type_Font)
		{
			headerDefines.push_back(fmt::format("#define {}\t{}", asset.Id, header.FontsCount));
			header.FontsCount += 1;
			size = LoadFont(asset, data);
			std::cout << fmt::format("{} -> Packing font [{:.3} KB] [{}]\n", asset.Id, size/1024.0f, asset.Path);
		}
		else if(asset.Type == Type_Wav)
		{
			headerDefines.push_back(fmt::format("#define {}\t{}", asset.Id, header.WavCount));
			header.WavCount += 1;
			size = LoadWav(asset, data);
			std::cout << fmt::format("{} -> Packing WAV [{:.3} KB] [{}]\n", asset.Id, size/1024.0f, asset.Path);
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

	std::string assetFileName = fmt::format("{}.dx1", arguments.Output); 
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
