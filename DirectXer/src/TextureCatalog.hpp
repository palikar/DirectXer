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

	void LoadTextures(Graphics* graphics, const char** paths, uint32 count);
	void LoadCubes(Graphics* graphics, const char** paths, uint32 count);

	BulkVector<LoadedTexture> LoadedTextures;
	BulkVector<LoadedTexture> LoadedCubes;
	
};
