#pragma once

#include <Types.hpp>
#include <GraphicsCommon.hpp>

enum Tag : uint16
{
	Tag_Level
};

enum AssetType : uint16
{
	Type_Image,
	Type_Font,
	Type_Wav,
};

struct AssetEntry
{
	uint64 Offset;
	uint64 Size;
	uint16 Type;
	uint16 Tag;
};

struct AssetColletionHeader
{
	uint32 ImagesCount;
	uint32 WavCount;
	uint32 FontsCount;	
	uint32 VersionSpec;	
};

struct ImageHeader
{
	uint16 Width;
	uint16 Height;
	TextureFormat Format;
};

struct WavAssetHeader
{
	uint32 Size;
	uint32 SampleRate;
	uint16 Channels;
	uint16 Bps;
};

struct FontHeader
{
	uint32 FontSize;
};



struct AtlasFileHeader
{
	uint16 NumAtlases;
	uint16 NumImages;
};

struct AtlasEntry
{
	int Width;
	int Height;
	TextureFormat Format;
	int Offset;
};

struct ImageEntry
{
	int Atlas;
	float Y;
	float X;
	float Width;
	float Height;
	float AtlasWidth;
	float AtlasHeight;
};
