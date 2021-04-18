#pragma once

#include <stb_image.h>
#include <fmt/format.h>
#include <string>

#include "Graphics.hpp"
#include "Memory.hpp"
#include "Utils.hpp"
#include "Platform.hpp"
#include "Resources.hpp"
#include "FileUtils.hpp"

enum LoadState
{
	LS_LOADED,
	LS_INVALID
};

struct TextureLoadEntry
{
	std::string_view Path;
	LoadState State{LS_INVALID};
	TextureId Handle{0};
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
	void LoadTextures(Graphics& graphics);

	TextureId LoadCube(Graphics& graphics, const char* name[6]);
};
