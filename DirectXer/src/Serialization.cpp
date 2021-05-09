#include <Lighting.hpp>
#include <Camera.hpp>
#include <Materials.hpp>

#include <FileUtils.hpp>
#include <Platform.hpp>
#include <Memory.hpp>

#include "Serialization.hpp"

void Serialization::DumpToFile(const char* path, SerializationContext& context)
{
	auto blobArena = Memory::GetTempArena(Megabytes(4));
	Defer { Memory::DestoryTempArena(blobArena); };

	SerializationHeader header;
	header.EntriesCount = 0;

	blobArena.Put(header);
	if (context.Camera)
	{
		((SerializationHeader*)blobArena.Memory)->EntriesCount += 1;
		blobArena.Put(SE_Camera);
		blobArena.Put(*context.Camera);
	}

	if (context.Lighting)
	{
		((SerializationHeader*)blobArena.Memory)->EntriesCount += 1;
		blobArena.Put(SE_Lighting);
		blobArena.Put(*context.Lighting);
	}

	for (uint32 i = 0; i < 8; ++i)
	{
		
		if (context.PhongMaterials[i])
		{
			((SerializationHeader*)blobArena.Memory)->EntriesCount += 1;
			blobArena.Put(SE_PhongMat);
			blobArena.Put(i);
			blobArena.Put(*context.PhongMaterials[i]);
		}

		if (context.TexturedMaterials[i])
		{
			((SerializationHeader*)blobArena.Memory)->EntriesCount += 1;
			blobArena.Put(SE_TexMat);
			blobArena.Put(*context.TexturedMaterials[i]);
		}

		if (context.MtlMaterials[i])
		{
			((SerializationHeader*)blobArena.Memory)->EntriesCount += 1;
			blobArena.Put(SE_MtlMat);
			blobArena.Put(*context.MtlMaterials[i]);
		}
		
	}


	DumpArenaToFile(path, blobArena);	
}

void Serialization::LoadFromFile(const char* path, SerializationContext& context)
{
	auto blobArena = Memory::GetTempArena(Megabytes(4));
	Defer { Memory::DestoryTempArena(blobArena); };

	ReadWholeFile(path, blobArena);
	auto current = blobArena.Memory;

	auto header = ReadBlob<SerializationHeader>(current);
	for (uint32 i = 0; i < header.EntriesCount; ++i)
	{
		auto entry = ReadBlob<SerializationEntry>(current);
		switch (entry) {
		  case SE_Camera:
			  *context.Camera = ReadBlob<Camera>(current);
			  break;
		  case SE_Lighting:
			  *context.Lighting = ReadBlob<Lighting>(current);
			  break;
		  case SE_PhongMat:
		  {
			  auto index = ReadBlob<uint32>(current);
			  *context.PhongMaterials[index] = ReadBlob<PhongMaterialData>(current);
			  break;
		  }
		  case SE_MtlMat:
		  {
			  auto index = ReadBlob<uint32>(current);
			  *context.MtlMaterials[index] = ReadBlob<MtlMaterialData>(current);
			  break;
		  }
		  case SE_TexMat:
		  {
			  auto index = ReadBlob<uint32>(current);
			  *context.TexturedMaterials[index] = ReadBlob<TexturedMaterialData>(current);
			  break;
		  }
		  default: continue;
		}
		
	}	
}
