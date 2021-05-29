#pragma once

#include <Serialization.hpp>
#include <GraphicsCommon.hpp>
#include <Types.hpp>
#include <Utils.hpp>
#include <Assets.hpp>
#include <Platform.hpp>
#include <Graphics.hpp>
#include <GraphicsCommon.hpp>
#include <Memory.hpp>
#include <Utils.hpp>
#include <Assets.hpp>
#include <FileUtils.hpp>
#include <Platform.hpp>
#include <TextureCatalog.hpp>
#include <2DRendering.hpp>

#include <imgui.h>

struct MaterialEditor
{
	struct CommandLineArguments
	{
		std::string Root{"resources"};
		std::string Input{"input.dxa"};
	};

};

struct MaterialEditEntry
{
	MaterialType Type;
	MaterialId Id;
	std::string Name;
	
	union {
		PhongMaterial* Phong;
		MtlMaterial* Mtl;
		TexturedMaterial* Tex;
	};
};

struct MeshEditEntry
{
	MeshId Id;
	float Scale{1.0f};
	glm::vec3 Position{0.0f, 0.0f, 0.0f};
};

struct LightHelpers
{
	bool Updating = true;

	float PointLight1Radius = 2.0f;
	float PointLight1Height = 0.0f;

	float PointLight2Radius = 2.0f;
	float PointLight2Height = 3.0f;

	float SpotHeight = 1.0f;
};

struct Context
{
	HWND hWnd;
	MaterialEditor::CommandLineArguments Args;
	bool FullscreenMode{false};
	UINT WindowStyle;
	RECT WindowRect;
	Graphics Graphics;
	float Width;
	float Height;

	Renderer3D Renderer3D;
	
	TextureCatalog Textures;

	std::vector<MeshEditEntry> Meshes;
	std::vector<const char*> MeshNames;

	std::vector<MaterialEditEntry> Materials;

	int CurrentMeshIndex{1};
	int CurrentMapIndex{1};
	int CurrentMaterialIndex{0};

	LightHelpers LightHelpersState;

	float T = 0.0f;
};

struct SessionSaveContext
{
	uint32 MaterialsCount;
	char* Paths;
};

void Init(Context& context);
void Update(Context& context, float dt);


// control camera (?)
// serialization
// Split up the tex and phong materials in their own shaders
