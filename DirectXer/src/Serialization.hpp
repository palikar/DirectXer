#pragma once

enum  SerializationEntry
{
	SE_Camera,
	SE_Lighting,

	SE_PhongMat,
	SE_TexMat,
	SE_MtlMat,
	
	SE_Count,
};

struct Camera;
struct Lighting;
struct PhongMaterialData;
struct PhongMaterialData;
struct TexturedMaterialData;
struct MtlMaterialData;

struct SerializationContext
{
	Camera* Camera;
	Lighting* Lighting;

	PhongMaterialData* PhongMaterials[8];
	TexturedMaterialData* TexturedMaterials[8];
	MtlMaterialData* MtlMaterials[8];
	
	
};

struct SerializationHeader
{
	uint32 EntriesCount;
};


struct Serialization
{
	static void DumpToFile(const char* path, SerializationContext& context);
	static void LoadFromFile(const char* path, SerializationContext& context);

};
