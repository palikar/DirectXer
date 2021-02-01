#pragma once

#include <stb_image.h>
#include <fmt/format.h>
#include <string>

#include "Graphics.hpp"

inline TextureFormat PngFormat(int channels)
{
	if(channels == 1) return TF_A;
	if(channels == 4) return TF_RGBA;

	return TF_UNKNOWN;
}

inline TextureObject LoadTexture(Graphics graphics, const char* t_Resources, const char* t_Name)
{
	auto path = fmt::format("{}/{}", t_Resources, t_Name);
	int width, height, channels;
	stbi_set_flip_vertically_on_load(1);
	unsigned char *data = stbi_load(path.c_str(), &width, &height, &channels, 0);
	auto textureHandle = graphics.createTexture(width, height, PngFormat(channels), data, width*height*channels);
	stbi_image_free(data);
	return textureHandle;

}

enum LoadState
{
	LS_LOADED,
	LS_INVALID
};

struct TextureLoadEntry
{
	std::string_view Path;
	LoadState State{LS_INVALID};
	TextureObject Handle{0};
};

inline TextureLoadEntry g_Textures[] 
{
	{"checker.png"},
};

#define CHECKER_TEXTURE g_Textures[0]


struct TextureCatalog
{
	void LoadTextures(Graphics graphics, std::string_view t_Resources)
	{
		
		const auto texturesCount = sizeof(g_Textures) / sizeof(TextureLoadEntry);
		fmt::basic_memory_buffer<char, 512> buf;

		for (size_t i = 0; i < texturesCount; ++i)
		{
			auto& tex = g_Textures[i];
			fmt::format_to(buf, "{}/{}", t_Resources, tex.Path);
			int width, height, channels;
			stbi_set_flip_vertically_on_load(1);
			
			unsigned char *data = stbi_load(buf.c_str(), &width, &height, &channels, 0);
			
			if (data == nullptr)
			{
				DXERROR("Can't load texture {} at {}. Reason: {}", tex.Path, buf.c_str(), stbi_failure_reason());
			}
			
			tex.Handle = graphics.createTexture(width, height, PngFormat(channels), data, width*height*channels);
			tex.State = LS_LOADED;
			stbi_image_free(data);
			buf.clear();
			
		}			
	}
	
};
