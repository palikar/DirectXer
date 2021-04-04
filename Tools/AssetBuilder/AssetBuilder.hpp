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
	const char* Id;
	ExtraData data{0};
};

size_t LoadImage(AssetToLoad asset, std::vector<unsigned char>& bytes);
size_t LoadWav(AssetToLoad asset, std::vector<unsigned char>& bytes);
size_t LoadFont(AssetToLoad asset, std::vector<unsigned char>& bytes);


