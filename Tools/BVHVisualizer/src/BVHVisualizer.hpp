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
#include <BVH.hpp>

#include <imgui.h>
#include <ImGuiFileDialog.h>
#include <filesystem>

namespace fs = std::filesystem;

struct BVHVisualizer
{
	struct CommandLineArguments
	{
		std::string Root{"resources"};
		std::string Input{"input.dxa"};
		bool fullScreen{false};
	};

};

struct Context
{
	HWND hWnd;
	BVHVisualizer::CommandLineArguments Args;
	bool FullscreenMode{false};
	UINT WindowStyle;
	RECT WindowRect;
	Graphics Graphics;
	float Width;
	float Height;
	bool Exit{false};

	TextureCatalog Textures;
	
	Renderer3D Renderer3D;

	float T = 0.0f;
	CameraControlState CameraState;

	SlowBVH Bvh;

	int BoxesToAdd{1};
	float3 MinBounds{-10.0f, -10.0f, -10.0f};
	float3 MaxBounds{10.0f, 10.0f, 10.0f};
	bool ShowBigBBox{true};

	size_t SelectedLeaf{0};
	int SelectedParent{0};

	bool ShowAllBoxes{true};
	
	bool ShowSelectedLeaf{false};
	bool ShowSelectedParent{false};

	bool OpenAllParents{false};
};

void Init(Context& context);
void Update(Context& context, float dt);
