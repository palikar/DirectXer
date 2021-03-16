#pragma once

#include <stb_image.h>
#include <fmt/format.h>
#include <string>

#include "Graphics.hpp"
#include "Memory.hpp"
#include "Utils.hpp"
#include "PlatformWindows.hpp"
#include "Resources.hpp"

inline TextureFormat PngFormat(int channels)
{
	if(channels == 1) return TF_A;
	if(channels == 4) return TF_RGBA;

	return TF_UNKNOWN;
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
	{"rocks_color.png"},
	{"floor_color.png"},
	{"rocks_ao.png"},
	{"rocks_normal.png"},
};

#define CHECKER_TEXTURE g_Textures[0]
#define ROCKS_TEXTURE g_Textures[1]
#define FLOOR_TEXTURE g_Textures[2]

#define ROCKS_AO_TEXTURE g_Textures[3]
#define ROCKS_NORMAL_TEXTURE g_Textures[4]

struct TextureCatalog
{

	void LoadTextures(Graphics graphics)
	{
		// @Note: We'll use this for loading the contents of the file
		MemoryArena fileArena = Memory::GetTempArena(Megabytes(16));

        // @Note: This will be used for the STB allocations
		Memory::EstablishTempScope(Megabytes(128));
		Defer { Memory::ResetTempMemory(); };
		
		const auto texturesCount = sizeof(g_Textures) / sizeof(TextureLoadEntry);

		stbi_set_flip_vertically_on_load(1);
		for (size_t i = 0; i < texturesCount; ++i)
		{
			fileArena.Reset();

			auto& tex = g_Textures[i];
			auto path = Resources::ResolveFilePath(tex.Path);

			DXLOG("[RES] Loading {}", path);

			PlatformLayer::ReadWholeFile(path, fileArena);

			int width, height, channels;
			unsigned char* data = stbi_load_from_memory((unsigned char*)fileArena.Memory, (int)fileArena.Size, &width, &height, &channels, 0);
			if (data == nullptr)
			{
				DXERROR("Can't load texture {} at {}. Reason: {}", tex.Path, path, stbi_failure_reason());
			}

			tex.Handle = graphics.createTexture(width, height, PngFormat(channels), data, width*height*channels);
			tex.State = LS_LOADED;
			
			Memory::ResetTempScope();
		}
		
	}

	TextureObject LoadCube(Graphics graphics, const char* name[6])
	{
		// @Note: We'll use this for loading the contents of the file
		MemoryArena fileArena = Memory::GetTempArena(Megabytes(16));

		// @Note: This will be used for the STB allocations
		Memory::EstablishTempScope(Megabytes(128));
		Defer { Memory::ResetTempMemory(); };

	

		int width, height, channels;
		// @Todo: Use temporary memory here
		void* data[6];
		stbi_set_flip_vertically_on_load(1);

		for (size_t i = 0; i < 6; ++i)
		{
			fileArena.Reset();

			auto& tex = name[i];
			auto path = Resources::ResolveFilePath(tex);
			DXLOG("[RES] Loading {}", path);

			PlatformLayer::ReadWholeFile(path, fileArena);

			data[i] = stbi_load_from_memory((unsigned char*)fileArena.Memory, (int)fileArena.Size, &width, &height, &channels, 0);

		}

		auto hand = graphics.createCubeTexture(width, height, PngFormat(channels), data);
		return hand;

	}

};
