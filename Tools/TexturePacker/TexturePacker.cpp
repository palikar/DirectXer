#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>
#include <fstream>
#include <GraphicsCommon.hpp>
#include <Types.hpp>
#include <Utils.hpp>
#include <Assets.hpp>
#include <fmt/format.h>
#include <filesystem>
#include <stb_image.h>
#include <stb_image_resize.h>
#include <stb_rect_pack.h>
#include <cmath>

#include "TexturePacker.hpp"

static inline ImageToPack ImagesToPack[] = {

	{"assets/evil_ship_1.png", 64, 64, "I_EVIL_SHIP_1"},
	{"assets/evil_ship_2.png", 64, 64, "I_EVIL_SHIP_2"},
	{"assets/evil_ship_3.png", 64, 64, "I_EVIL_SHIP_3"},
	{"assets/PNG/Ship_Parts/Ship_Main_Icon.png", 64, 64, "I_MAIN_SHIP"},
	{"assets/ship_bullet.png", 32, 64, "I_BULLET"},
	{"assets/explosion.png", 0, 0, "I_EXPLOSION"},
	{"assets/heart.png", 32, 32, "I_HEART"},
	{"assets/PNG/Main_UI/Health_Bar_Table.png", 0, 0, "I_HEALTH"},
};

struct CommandLineArguments
{
	std::string Root{"resources"};
	std::string Output{"output"};
	uint16 Size{1024};
	uint16 MaxAtlases{20};
};

static void ParseCommandLineArguments(int argc, char *argv[], CommandLineArguments& arguments)
{
	for (size_t i = 0; i < argc; ++i)
	{
		std::string current{argv[i]};
		if(current == "-r") {
			arguments.Root = argv[++i];
		} else if (current == "-o") {
			arguments.Output = argv[++i];
		} else if (current == "-s") {
			arguments.Size = std::stoi(argv[++i]);
		} else if (current == "-m") {
			arguments.MaxAtlases = std::stoi(argv[++i]);
		}
	}
}

int main(int argc, char *argv[])
{
	CommandLineArguments arguments{};
	ParseCommandLineArguments(argc, argv, arguments);

	AtlasFileHeader header{0};
	header.NumAtlases = 1;
	header.NumImages = (uint16)size(ImagesToPack);

	uint16 atlasSize = arguments.Size;
	uint16 maxAtlases = arguments.MaxAtlases;

	std::vector<ImageEntry> images;
	images.reserve(50);

	std::vector<AtlasEntry> atlases;
	images.reserve(3);

	std::vector<std::vector<unsigned char>> atlasesBytes(1);
	atlasesBytes.back().resize(atlasSize * atlasSize * 4);

	AtlasEntry atlasEntry;
	atlasEntry.Width = atlasSize;
	atlasEntry.Height = atlasSize;
	atlasEntry.Format = TF_RGBA;
	atlasEntry.Offset = sizeof(AtlasFileHeader) + sizeof(ImageEntry)*header.NumImages + sizeof(AtlasEntry);
	atlases.push_back(atlasEntry);

	std::vector<stbrp_context> RectContexts;
	RectContexts.resize(maxAtlases);
	stbrp_init_target(&RectContexts[0], atlasSize, atlasSize, (stbrp_node*)malloc(sizeof(stbrp_node)*atlasSize), atlasSize);
	
	fmt::print("Packing [{}] images\n", size(ImagesToPack));
	for (size_t i = 0; i < size(ImagesToPack); ++i)
	{
		auto& entry = ImagesToPack[i];
		entry.Path = fmt::format("{}/{}", arguments.Root, entry.Path);

		int width, height, channels;
		unsigned char* data = stbi_load(entry.Path.c_str(), &width, &height, &channels, 4);

		if(!data)
		{
			fmt::print("Cannot open image file: [{}]\n", entry.Path);
			return 1;
		}

		unsigned char* resized = data;
		if (entry.Width != 0 && (width != entry.Width || height != entry.Height))
		{
			auto res = stbir_resize_uint8(data, width, height, 0,
										  resized, entry.Width, entry.Height, 0,
										  4);
			width = entry.Width;
			height = entry.Height;
		}

		stbrp_rect rect;
		rect.w = (stbrp_coord)width;
		rect.h = (stbrp_coord)height;
		ImageEntry image;
		strcpy_s(image.Id, entry.Id);
				
		for (size_t j = 0; j < maxAtlases; ++j)
		{
			if (j >= atlasesBytes.size())
			{
				uint16 atlasWidth = std::max(atlasSize, (uint16)width);
				uint16 atlasHeight = std::max(atlasSize, (uint16)height);
				fmt::print("Generating a new atlas [{}] with size [{}x{}]!\n", j, atlasWidth, atlasHeight);
				stbrp_init_target(&RectContexts[j], atlasWidth, atlasWidth, (stbrp_node*)malloc(sizeof(stbrp_node)*atlasSize), atlasSize);
				atlasesBytes.push_back({});
				atlasesBytes.back().resize(atlasSize * atlasSize * 4);
				atlasEntry.Offset += atlasHeight * atlasWidth * 4;
				atlasEntry.Width = atlasWidth;
				atlasEntry.Height = atlasHeight;
				atlases.push_back(atlasEntry);
				std::for_each(atlases.begin(), atlases.end(), [](auto& atlas){ atlas.Offset +=  sizeof(AtlasEntry); });
				header.NumAtlases += 1;
			}
			
			stbrp_pack_rects(&RectContexts[j], &rect, 1);
			image.Atlas = (uint16)j;
			if (rect.was_packed != 0) break;
		}

		if (rect.was_packed == 0)
		{
			fmt::print("Cannot pack image : [{}]. Maybe increase the maximum number of atlases\n", entry.Path);
			return 1;
		};
		
		fmt::print("Packing image with size [{}x{}] in atlas [{}] : [{}]\n", width, height, image.Atlas, entry.Path);
		
		image.X = rect.x / (float)atlases[image.Atlas].Width;
		image.Y = rect.y / (float)atlases[image.Atlas].Height;
		image.Width = rect.w / (float)atlases[image.Atlas].Width;
		image.Height = rect.h / (float)atlases[image.Atlas].Height;
		image.AtlasWidth = (float)atlases[image.Atlas].Width;
		image.AtlasHeight = (float)atlases[image.Atlas].Height;

		images.push_back(image);

		PutPixels(atlasesBytes[image.Atlas], rect, resized, atlases[image.Atlas].Width);
		
		stbi_image_free(data);
	}

	
	std::string atlasFileName = fmt::format("{}.datlas", arguments.Output); 
	std::ofstream outfile(atlasFileName, std::ios::out | std::ios::binary);
	if (!outfile.is_open())
	{
		fmt::print("Cannot open output file: [{}]\n", atlasFileName);
		return 1;
	}

	fmt::print("Generating output file [{}]\n", atlasFileName);

	outfile.write((char*)&header, sizeof(AtlasFileHeader));
	outfile.write((char*)atlases.data(), atlases.size() * sizeof(AtlasEntry));
	outfile.write((char*)images.data(), images.size() * sizeof(ImageEntry));
	for (auto& bytes : atlasesBytes)
	{
		outfile.write((char*)bytes.data(), bytes.size() * sizeof(char));
	}

	
	return 0;
}
