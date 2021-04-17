#pragma once

#include <Types.hpp>
#include <GraphicsCommon.hpp>
#include <Memory.hpp>

enum Tag : uint16
{
	Tag_Level
};

enum AssetType : uint16
{
	Type_Image,
	Type_Font,
	Type_Wav,
	Type_Atlas,
};

struct AssetEntry
{
	uint64 Offset;
	uint64 Size;
	AssetType Type;
	uint16 Tag;
};

struct AssetColletionHeader
{
	uint32 ImagesCount;
	uint32 WavCount;
	uint32 FontsCount;	
	uint32 AtlasesCount;
	uint32 VersionSpec;	
};

struct ImageHeader
{
	uint32 Id;
	uint16 Width;
	uint16 Height;
	TextureFormat Format;
};

struct WavAssetHeader
{
	uint32 Size;
	uint32 SampleRate;
	uint32 Format;
	uint16 Channels;
	uint16 Bps;
};

struct FontHeader
{
	uint32 FontSize;
	uint32 Id;
	uint32 DataSize;
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
	char Name[64];
	uint32 Id;
	uint32 Atlas;
	float Y;
	float X;
	float Width;
	float Height;
	float AtlasWidth;
	float AtlasHeight;
};

class ImageLibrary;
class AudioPlayer;
class FontLibrary;

struct AssetBuilder
{
	ImageLibrary* ImageLib;
	FontLibrary* FontLib;
	AudioPlayer* WavLib;
};

struct AssetStore
{
	static void LoadAssetFile(String path, AssetBuilder& builders);
};


