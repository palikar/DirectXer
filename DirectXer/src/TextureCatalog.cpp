#include <TextureCatalog.hpp>

#include <stb_image.h>


void TextureCatalog::LoadTextures(Graphics& graphics)
{
	// @Note: We'll use this for loading the contents of the file
	MemoryArena fileArena = Memory::GetTempArena(Megabytes(16));

	// @Note: This will be used for the STB allocations
	Memory::EstablishTempScope(Megabytes(128));
	Defer {
		Memory::EndTempScope();
		Memory::DestoryTempArena(fileArena);
	};


	const auto texturesCount = sizeof(g_Textures) / sizeof(TextureLoadEntry);

	stbi_set_flip_vertically_on_load(1);
	for (size_t i = 0; i < texturesCount; ++i)
	{
		fileArena.Reset();

		auto& tex = g_Textures[i];
		auto path = Resources::ResolveFilePath(tex.Path);

		DXLOG("[RES] Loading {}", path);

		ReadWholeFile(path, fileArena);

		int width, height, channels;
		unsigned char* data = stbi_load_from_memory((unsigned char*)fileArena.Memory, (int)fileArena.Size, &width, &height, &channels, 4);
		if (data == nullptr)
		{
			DXERROR("Can't load texture {} at {}. Reason: {}", tex.Path, path, stbi_failure_reason());
		}

		tex.Handle = NextTextureId();
		graphics.CreateTexture(tex.Handle, {(uint16)width, (uint16)height, TF_RGBA}, data);
		tex.State = LS_LOADED;

		Memory::ResetTempScope();
	}
}

TextureId TextureCatalog::LoadCube(Graphics& graphics, const char* name[6])
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

		ReadWholeFile(path, fileArena);

		data[i] = stbi_load_from_memory((unsigned char*)fileArena.Memory, (int)fileArena.Size, &width, &height, &channels, 4);

	}

	const auto texId = NextTextureId();
	graphics.CreateCubeTexture(texId, {(uint16)width, (uint16)height, TF_RGBA}, data);
	return texId;

}
