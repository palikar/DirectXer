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


//@Note: as we don't have different structs for the different types of entries
// (and we probably should at some point), each entry can have some extra information
// that is context dependent based on the type of the asset to be packaged; this structure
// stores this extra data
struct ExtraData
{
	uint32 unsigned1;
	uint32 unsigned2;
	int integet1;
	int integet2;
	float float1;
	float float2;
};

struct AssetToLoad
{
	AssetType Type;
	Tag TagField;
	std::string Path;

	// @Note: this will be the symbol's name with which the C++ code will
	// refer to this asset
	const char* Id;

	ExtraData data{0};
};

size_t LoadImage(AssetToLoad asset, std::vector<unsigned char>& bytes, uint32 id);
size_t LoadWav(AssetToLoad asset, std::vector<unsigned char>& bytes);
size_t LoadFont(AssetToLoad asset, std::vector<unsigned char>& bytes, uint32 id);
size_t LoadAtlas(AssetToLoad asset, std::vector<unsigned char>& bytes,
				 std::vector<std::string>& headerDefines, AssetColletionHeader& header);
