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

inline static void PutPixels(std::vector<unsigned char>& atlasBytes, stbrp_rect rect, unsigned char* data, size_t atlasSize)
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
