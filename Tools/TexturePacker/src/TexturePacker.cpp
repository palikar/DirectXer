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

static inline ImageToPack images[] = {

	{"assets/evil_ship_1.png", 64, 64, "I_EVIL_SHIP_1"},
	{"assets/evil_ship_2.png", 64, 64, "I_EVIL_SHIP_2"},
	{"assets/evil_ship_3.png", 64, 64, "I_EVIL_SHIP_3"},
	{"assets/PNG/Ship_Parts/Ship_Main_Icon.png", 64, 64, "I_MAIN_SHIP"},
	{"assets/ship_bullet.png", 32, 64, "I_BULLET"},
	{"assets/explosion.png", 0, 0, "I_EXPLOSION"},
	{"assets/heart.png", 32, 32, "I_HEART"},
	{"assets/PNG/Main_UI/Health_Bar_Table.png", 0, 0, "I_HEALTH"},
};

static void ParseCommandLineArguments(int argc, char *argv[], TexturePacker::CommandLineArguments& arguments)
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

static void PutPixels(std::vector<unsigned char>& atlasBytes, stbrp_rect rect, unsigned char* data, size_t atlasSize)
{

	for (size_t i = 0; i < rect.h; ++i)
	{
		for (size_t j = 0; j < rect.w; ++j)
		{
			atlasBytes[((i + rect.y) * atlasSize + (j + rect.x))*4 + 0] = data[(i*rect.w + j)*4 + 0];
			atlasBytes[((i + rect.y) * atlasSize + (j + rect.x))*4 + 1] = data[(i*rect.w + j)*4 + 1];
			atlasBytes[((i + rect.y) * atlasSize + (j + rect.x))*4 + 2] = data[(i*rect.w + j)*4 + 2];
			atlasBytes[((i + rect.y) * atlasSize + (j + rect.x))*4 + 3] = data[(i*rect.w + j)*4 + 3];
		}
	}	
}

TexturePackerOutput PackTextures(TexturePacker::CommandLineArguments arguments, ImageToPack* imagesToPack, size_t imagesCount)
{
	
	AtlasFileHeader header{0};
	header.NumAtlases = 1;
	header.NumImages = (uint16)imagesCount;

	uint16 atlasSize = arguments.Size;
	uint16 maxAtlases = arguments.MaxAtlases;

	std::vector<AtlasImage> images;
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

	fmt::print("Packing [{}] images\n", imagesCount);
	
	for (size_t i = 0; i < imagesCount; ++i)
	{
		auto& entry = imagesToPack[i];
		entry.Path = fmt::format("{}/{}", arguments.Root, entry.Path);

		int width, height, channels;
		unsigned char* data = stbi_load(entry.Path.c_str(), &width, &height, &channels, 4);

		if(!data)
		{
			fmt::print("Cannot open image file: [{}]\n", entry.Path);
			return {};
		}

		unsigned char* resized = data;
		if (entry.Width != 0 && (width != entry.Width || height != entry.Height))
		{
			auto res = stbir_resize_uint8(data, width, height, 0, resized, entry.Width, entry.Height, 0, 4);
			width = entry.Width;
			height = entry.Height;
		}

		stbrp_rect rect;
		rect.w = (stbrp_coord)width;
		rect.h = (stbrp_coord)height;
		AtlasImage image;
		strcpy_s(image.Name, entry.Id);
		image.Id = 0;
				
		for (size_t j = 0; j < maxAtlases; ++j)
		{
			if (j >= atlasesBytes.size())
			{
				uint16 atlasWidth = std::max(atlasSize, (uint16)(width + 16));
				uint16 atlasHeight = std::max(atlasSize, (uint16)(height + 16));
				fmt::print("Generating a new atlas [{}] with size [{}x{}]!\n", j, atlasWidth, atlasHeight);
				stbrp_init_target(&RectContexts[j], atlasWidth, atlasHeight, (stbrp_node*)malloc(sizeof(stbrp_node)* atlasWidth), atlasSize);
				atlasesBytes.push_back({});
				atlasesBytes.back().resize(atlasWidth * atlasHeight * 4);
				atlasEntry.Offset += atlasHeight * atlasWidth * 4;
				atlasEntry.Width = atlasWidth;
				atlasEntry.Height = atlasHeight;
				atlases.push_back(atlasEntry);
				std::for_each(atlases.begin(), atlases.end(), [](auto& atlas){ atlas.Offset +=  sizeof(AtlasEntry); });
				header.NumAtlases += 1;
			}
			
			stbrp_pack_rects(&RectContexts[j], &rect, 1);
			image.Atlas = (uint32)j;
			if (rect.was_packed != 0) break;
		}

		if (rect.was_packed == 0)
		{
			fmt::print("Cannot pack image : [{}]. Maybe increase the maximum number of atlases\n", entry.Path);
			return {};
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

	TexturePackerOutput res;
	res.Header = header;
	res.Images = std::move(images);
	res.Atlases = std::move(atlases);
	res.AtlasesBytes = std::move(atlasesBytes);

	return res;
}

#ifdef TEXTURE_PACKER_COMPILE_APPLICATION
int main(int argc, char *argv[])
{
	TexturePacker::CommandLineArguments arguments{};
	ParseCommandLineArguments(argc, argv, arguments);

	TexturePackerOutput res = PackTextures(arguments, images, size(images));
		
	std::string atlasFileName = fmt::format("{}.datlas", arguments.Output); 
	std::ofstream outfile(atlasFileName, std::ios::out | std::ios::binary);
	if (!outfile.is_open())
	{
		fmt::print("Cannot open output file: [{}]\n", atlasFileName);
		return 1;
	}

	fmt::print("Generating output file [{}]\n", atlasFileName);

	outfile.write((char*)&res.Header, sizeof(AtlasFileHeader));
	outfile.write((char*)res.Atlases.data(), res.Atlases.size() * sizeof(AtlasEntry));
	outfile.write((char*)res.Images.data(), res.Images.size() * sizeof(ImageEntry));
	for (auto& bytes : res.AtlasesBytes)
	{
		outfile.write((char*)bytes.data(), bytes.size() * sizeof(char));
	}
	
	return 0;
}
#endif
