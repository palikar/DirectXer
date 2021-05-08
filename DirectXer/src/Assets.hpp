#pragma once

#include <Types.hpp>
#include <GraphicsCommon.hpp>
#include <Memory.hpp>
#include <ImageLibrary.hpp>
#include <Audio.hpp>
#include <FontLibrary.hpp>
#include <GeometryUtils.hpp>

enum Tag : uint16
{
	Tag_Level
};

struct AssetColletionHeader
{
	uint32 TexturesCount;
	uint32 IBsCount;
	uint32 VBsCount;

	uint32 ImagesCount;
	uint32 AtlasesCount;
	uint32 SkyboxesCount;

	uint32 LoadImagesCount;
	uint32 LoadWavsCount;
	uint32 LoadFontsCount;

	uint32 LoadMeshesCount;
	
	uint32 VersionSpec;	
};

struct TextureLoadEntry
{
	TextureDescription Desc;
	TextureId Id;
	size_t DataOffset;
};

struct IBLoadEntry
{
	uint32 DataSize;
	bool Dynamic;
	
	size_t DataOffset;
	IndexBufferId Id;
};

struct VBLoadEntry
{
	uint32 StructSize;
	uint32 DataSize;
	bool Dynamic;

	size_t DataOffset;
	VertexBufferId Id;
};

struct SkyboxLoadEntry
{
	TextureDescription Desc;
	TextureId Id;
	size_t DataOffset[6];
};

// @Note: Will be put directly into the images map of the ImageLibrary
struct ImageEntry
{
	Image Image;
	ImageId Id;
};

// @Note: Will be used to create image from memory
struct ImageLoadEntry
{
	ImageDescription Desc;
	ImageId Id;
	size_t DataOffset;
};

// @Note: Will be used to create WAV from memory
struct WavLoadEntry
{
	WavDescription Desc;
	WavId Id;
	size_t DataOffset;
};

// @Note: Will be used to create font from memory
struct FontLoadEntry
{
	FontDescription Desc;
	FontId Id;
	size_t DataOffset;
	size_t DataSize;
};

struct MeshLoadEntry
{
	MeshGeometryInfo Mesh;
	uint32 Id;
};

struct AtlasFileHeader
{
	uint16 NumAtlases;
	uint16 NumImages;
};

class ImageLibrary;
class AudioPlayer;
class FontLibrary;

struct AssetBuildingContext
{
	ImageLibrary* ImageLib;
	FontLibrary* FontLib;
	AudioPlayer* WavLib;
	MeshCatalog* MeshesLib;

	Graphics* Graphics;
};

struct AssetFile
{
	const char* Path;
	size_t Size;
};

struct AssetStore
{
	static void LoadAssetFile(AssetFile file, AssetBuildingContext& builders);
};
