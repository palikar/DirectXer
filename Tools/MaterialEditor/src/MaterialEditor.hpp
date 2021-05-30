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
#include <ImGuiFileDialog.h>
#include <filesystem>

namespace fs = std::filesystem;

struct MaterialEditor
{
	struct CommandLineArguments
	{
		std::string Root{"resources"};
		std::string Input{"input.dxa"};

		bool fullScreen{false};
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
	bool Exit{false};

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

inline static MaterialId NextMaterialId = 1;

void LoadSession(Context& context, const char* path);
MtlMaterial LoadMtlMaterial(Context& context, fs::path path);
PhongMaterial LoadPhongMaterial(Context& context, fs::path path);
TexturedMaterial LoadTexturedMaterial(Context& context, fs::path path);
void LoadMaterial(Context& context, fs::path path);
void LoadSession(Context& context, const char* path);

void SaveMaterial(Context& context, MtlMaterial mat, const char* path, const char* name);
void SaveMaterial(Context& context, PhongMaterial mat, const char* path, const char* name);
void SaveMaterial(Context& context, TexturedMaterial mat, const char* path, const char* name);
void SaveSession(Context& context, const char* path);
