#pragma once

#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>

#include <GraphicsCommon.hpp>
#include <Types.hpp>
#include <Utils.hpp>
#include <Assets.hpp>

#include <fmt/format.h>
#include <stb_image.h>

namespace fs = std::filesystem;

//@Note: as we don't have different structs for the different types of entries
// (and we probably should at some point), each entry can have some extra information
// that is context dependent based on the type of the asset to be packaged; this structure
// stores this extra data
struct ExtraData
{
	uint32 unsigned1;
	uint32 unsigned2;
	int integet1;
	int integet2;
	float float1;
	float float2;
};

enum AssetType : uint16
{
	// @Note: This ordering is important! it should match the one of
	// GPUResourceType in Assets.hpp
	Type_Texture,
	Type_VertexBuffer,
	Type_IndexBuffer,
	Type_ConstantBuffer,
	
	Type_Image,
	Type_Font,
	Type_Wav,
	Type_Skybox,
	Type_Mesh,
	Type_Material,
};

struct AssetToLoad
{
	AssetType Type;
	Tag TagField;
	std::string Path;

	// @Note: this will be the symbol's name with which the C++ code will
	// refer to this asset
	const char* Id;

	ExtraData data{0};
};

struct AssetBuilder
{
	struct CommandLineArguments
	{
		std::string Root{""};
		std::string Output{"output"}; 
		std::string Header{"output"};
		std::string Id{"Asset"};
		size_t MaxSize{128};
	};

};

struct AssetDefine
{
	std::string Name;
	uint32 Id;
	AssetType Type;
};

struct AssetBundlerContext
{
	AssetColletionHeader Header;
	std::vector<AssetDefine> Defines;
	
	// @Note: Those objects will be created through the Graphics
	std::vector<TextureLoadEntry> TexturesToCreate;
	std::vector<VBLoadEntry> VBsToCreate;
	std::vector<IBLoadEntry> IBsToCreate;

	// @Note: Those images and atlases are purely meta data which 
	// will be inserte into the ImageLibrary's images map; the textures will
	// be created with the "TexturesToCreate" entries
	std::vector<ImageEntry> Images;
	std::vector<ImageAtlas> Atlases;

	std::vector<MaterialLoadEntry> Materials;

	std::vector<SkyboxLoadEntry> Skyboxes;
	
	// @Note: These entries have some actual byte data that will be used
	// to create the corresponding object
	std::vector<ImageLoadEntry> LoadImages;
	std::vector<WavLoadEntry> LoadWavs;
	std::vector<FontLoadEntry> LoadFonts;

	std::vector<MeshLoadEntry> LoadMeshes;

	// @Note: The byte data of everything above
	std::vector<unsigned char> Data;
};

struct AssetDataBlob
{
	std::vector<unsigned char> Data;
	uint64 CurrentOffset{ 0 };

	// @Note: Will keep track of the last inserted size; used for debugging
	// and logging porposes
	uint64 lastSize;

	uint64 PutData(std::vector<unsigned char>& newData, size_t offset = 0)
	{
		auto res = CurrentOffset;
		CurrentOffset += newData.size() - offset;
		lastSize = newData.size() - offset;
		//Data.insert(Data.end()
		//, newData.begin() + offset, newData.end());
		
		auto oldSize = Data.size();
		Data.resize(Data.size() + newData.size() - offset);
		memcpy(Data.data() + oldSize, newData.data() + offset, newData.size() - offset);
		
		return res;
	}

	uint64 PutData(unsigned char* newData, size_t size)
	{
		auto res = CurrentOffset;
		CurrentOffset += size;
		lastSize = size;
	
		// @Note: this make the code super fast
		auto oldSize = Data.size();
		Data.resize(Data.size() + size);
		memcpy(Data.data() + oldSize, newData, size);
		
		return res;
	}
};

void LoadImage(AssetToLoad asset, AssetBundlerContext& context, AssetDataBlob& blob);
void LoadWav(AssetToLoad asset, AssetBundlerContext& context, AssetDataBlob& blob);
void LoadFont(AssetToLoad asset, AssetBundlerContext& context, AssetDataBlob& blob);
void LoadTexture(AssetToLoad asset, AssetBundlerContext& context, AssetDataBlob& blob);
void LoadSkybox(AssetToLoad asset, AssetBundlerContext& context, AssetDataBlob& blob);
void LoadMesh(AssetToLoad asset, AssetBundlerContext& context, AssetDataBlob& blob);
void LoadMaterial(AssetToLoad asset, AssetBundlerContext& context, AssetDataBlob& blob);

uint32 LoadDefaultMaterial(AssetBundlerContext& context);
	
inline uint32 NewAssetName(AssetBundlerContext& context, AssetType type, const char* name, uint32 id = 0) 
{
	static uint32 next = 0;
	uint32 nextId = (id == 0 ?  ++next : id);
	context.Defines.push_back({ name, nextId, type });
	return nextId;
}

inline TextureId NextTextureAssetId()
{
	static TextureId next = 0;
	++next;
	return 1 << 15 | next;
}

inline VertexBufferId NextVBAssetId()
{
	static VertexBufferId next = 0;
	++next;
	return 1 << 15 | next;
}

inline IndexBufferId NextIBAssetId()
{
	static IndexBufferId next = 0;
	++next;
	return 1 << 15 | next;
}

inline ConstantBufferId NextCBAssetId()
{
	static ConstantBufferId next = 0;
	++next;
	return 1 << 15 | next;
}
