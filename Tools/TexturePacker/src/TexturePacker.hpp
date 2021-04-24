#pragma once

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


template<class T, size_t N>
constexpr size_t size(T (&)[N])
{
	return N;
}

struct ImageToPack
{
	std::string Path;
	int Width;
	int Height;
	const char* Id;
};

struct TexturePacker
{
	struct CommandLineArguments
	{
		std::string Root{"resources"};
		std::string Output{"output"};
		uint16 Size{1024};
		uint16 MaxAtlases{20};
	};
	
};

struct AtlasImage
{
	char Name[64];
	uint32 Id;
	uint32 Atlas;
	float Y;
	float X;
	float Width;
	float Height;
	float AtlasWidth;
	float AtlasHeight;
};

struct AtlasEntry
{
	int Width;
	int Height;
	TextureFormat Format;
	int Offset;
	TextureId Id;
};


struct TexturePackerOutput
{
	AtlasFileHeader Header;
	std::vector<AtlasImage> Images;
	std::vector<AtlasEntry> Atlases;
	std::vector<std::vector<unsigned char>> AtlasesBytes;
};

TexturePackerOutput PackTextures(TexturePacker::CommandLineArguments arguments, ImageToPack* imagesToPack, size_t imagesCount);
