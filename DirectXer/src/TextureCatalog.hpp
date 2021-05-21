#pragma once

#include <Graphics.hpp>
#include <Memory.hpp>
#include <Utils.hpp>
#include <Platform.hpp>
#include <Resources.hpp>
#include <FileUtils.hpp>

#include <stb_image.h>
#include <fmt/format.h>

enum LoadState
{
	LS_LOADED,
	LS_INVALID
};

struct TextureLoadEntry_
{
	String Path;
	LoadState State{LS_INVALID};
	TextureId Handle{0};
};

struct LoadedTexture
{
	String Path;
	String Name;
	TextureId Handle;
};

inline TextureLoadEntry_ g_Textures[]
{
	{"checker.png"},
	{"rocks_color.png"},
	{"floor_color.png"},
	{"rocks_ao.png"},
	{"rocks_normal.png"},
};

struct TextureCatalog
{
	void LoadTextures(Graphics& graphics);
	
	TextureId LoadCube(Graphics& graphics, const char* name[6]);

	void LoadTextures(Graphics* graphics, const char** paths, uint32 count)
	{
		// @Note: We'll use this for loading the contents of the file
		MemoryArena fileArena = Memory::GetTempArena(Megabytes(16));

		// @Note: This will be used for the STB allocations
		Memory::EstablishTempScope(Megabytes(128));
		Defer {
			Memory::EndTempScope();
			Memory::DestoryTempArena(fileArena);
		};

		for (uint32 i = 0; i < count; ++i)
		{
			fileArena.Reset();

			DXLOG("[RES] Loading {}", paths[i]);

			ReadWholeFile(paths[i], fileArena);

			int width, height, channels;
			unsigned char* data = stbi_load_from_memory((unsigned char*)fileArena.Memory, (int)fileArena.Size, &width, &height, &channels, 4);
			if (data == nullptr)
			{
				DXERROR("Can't load texture {}. Reason: {}", paths[i], stbi_failure_reason());
			}

			LoadedTexture newTex;
			newTex.Handle = NextTextureId();
			newTex.Path = paths[i];
			newTex.Name = paths[i];

			
			graphics->CreateTexture(newTex.Handle, {(uint16)width, (uint16)height, TF_RGBA}, data);
			Memory::ResetTempScope();
			LoadedTextures.push_back(newTex);
		}
	}

	void LoadCubes(Graphics* graphics, const char** paths, uint32 count)
	{
		// @Note: We'll use this for loading the contents of the file
		MemoryArena fileArena = Memory::GetTempArena(Megabytes(16));

		// @Note: This will be used for the STB allocations
		Memory::EstablishTempScope(Megabytes(128));
		Defer { Memory::ResetTempMemory(); };

		TempFormater formater;
		for (uint32 i = 0; i < count; ++i)
		{
			int width, height, channels;
			void* data[6];
			stbi_set_flip_vertically_on_load(1);

			const char* names[] = { "left", "right","up", 
				"down", "front", "back"
			};

			const char* extensions[] = {"png", "jpg"};
			
			for (size_t j = 0; j < 6; ++j)
			{
				for (size_t k = 0; k < 2; ++k)
				{
					fileArena.Reset();
					auto path = formater.Format("{}/{}.{}", paths[i], names[j], extensions[k]);
					if (!PlatformLayer::IsValidPath(path)) continue;

					ReadWholeFile(path, fileArena);

					data[j] = stbi_load_from_memory((unsigned char*)fileArena.Memory, (int)fileArena.Size, &width, &height, &channels, 4);
				}
			}

			LoadedTexture newTex;
			newTex.Handle = NextTextureId();
			newTex.Path = paths[i];
			newTex.Name = paths[i];

			graphics->CreateCubeTexture(newTex.Handle, {(uint16)width, (uint16)height, TF_RGBA}, data);
			LoadedCubes.push_back(newTex);
		}

	
	}

	BulkVector<LoadedTexture> LoadedTextures;
	BulkVector<LoadedTexture> LoadedCubes;
	
};
