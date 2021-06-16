
#include "BVHVisualizer.hpp"

#include <Random.hpp>

namespace fs = std::filesystem;

static uint32 CUBE_RED;
static uint32 CUBE_GREEN;
static uint32 CUBE_BLUE;

static uint32 PLANE;
static uint32 LINES;
static uint32 CYLINDER;
static uint32 SPHERE;
static uint32 AXIS;
static uint32 POINTLIGHT;
static uint32 SPOTLIGHT;

const static float pov = 65.0f;
const static float nearPlane = 0.0001f;
const static float farPlane = 10000.0f;


void Init(Context& context)
{
	auto Graphics = &context.Graphics;
	context.Renderer3D.InitRenderer(Graphics);

	Memory::EstablishTempScope(Megabytes(4));
	{
		DebugGeometryBuilder builder;
		builder.Init(8);
		CUBE_RED = builder.InitCube(CubeGeometry{}, float3{ 1.0f, 0.0f, 0.0f });
		CUBE_GREEN = builder.InitCube(CubeGeometry{}, float3{ 0.0f, 1.0f, 0.0f });
		CUBE_BLUE = builder.InitCube(CubeGeometry{}, float3{ 0.0f, 0.0f, 1.0f });


		PLANE = builder.InitPlane(PlaneGeometry{}, float3{ 0.0f, 1.0f, 0.0f });
		SPHERE = builder.InitSphere(SphereGeometry{}, float3{ 0.0f, 1.0f, 0.0f });
		CYLINDER = builder.InitCylinder(CylinderGeometry{ 0.25, 0.25, 1.5 }, float3{ 1.0f, 1.0f, 0.0f });
		LINES = builder.InitLines(LinesGeometry{}, float3{ 0.8f, 0.8f, 0.8f });
		AXIS = builder.InitAxisHelper();
		POINTLIGHT = builder.InitPointLightHelper();
		SPOTLIGHT = builder.InitSpotLightHelper();

		context.Renderer3D.InitDebugGeometry(builder);
	}
	Memory::EndTempScope();


	const char* Envs[] = {
		"resources/textures/cubes/sky",
		"resources/textures/cubes/night_sky",
	};

	context.Textures.LoadedCubes.reserve(4);
	context.Textures.LoadCubes(Graphics, Envs, (uint32)Size(Envs));

	context.Renderer3D.CurrentCamera.Pos = { 1.0f, 0.5f, 1.0f };
	context.Renderer3D.CurrentCamera.lookAt({ 0.0f, 0.0f, 0.0f });
	context.Renderer3D.SetupProjection(glm::perspective(pov, 1080.0f / 720.0f, nearPlane, farPlane));

	context.Renderer3D.MeshData.Materials.Init();
	context.Renderer3D.InitLighting();

	context.Bvh.Nodes.reserve(2048);
	
}

static void DrawAABB(Renderer3D& Renderer, AABB aabb, uint32 cube = 0)
{
	float3 size = glm::abs(aabb.Max - aabb.Min);
	float3 pos = (glm::min(aabb.Min, aabb.Max)) + size/2.0f;

	Renderer.DrawDebugGeometry(cube, pos, size);
}

static float RandomFloat(float a, float b)
{
	return Random::Uniform(std::min(a, b), std::max(a, b));
}

static void DisplayParentNode(Context& context, int parent)
{
	auto& parenNode = context.Bvh.Nodes[parent];

	if (parenNode.child1 == -1 && parenNode.child2 == -1)
	{
		// this is a leaf node!
		TempFormater formater;
		if (ImGui::Selectable(formater.Format("-Box: {}", parent), context.SelectedParent == parent))
		{
			context.SelectedParent = parent;
		}
		
		return;
	 }
	
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
		| ImGuiTreeNodeFlags_OpenOnDoubleClick
		| ImGuiTreeNodeFlags_SpanAvailWidth
		| ImGuiTreeNodeFlags_OpenOnArrow;
	flags |= (context.SelectedParent == parent ? ImGuiTreeNodeFlags_Selected : 0);

	ImGui::SetNextItemOpen(context.OpenAllParents, ImGuiCond_None);
	bool parentOpened = ImGui::TreeNodeEx((void*)(intptr_t)parent, flags, "Box %d", parent);
	if (ImGui::IsItemClicked()) context.SelectedParent = parent;

	if (parentOpened)
	{
		if (parenNode.child1 != -1) DisplayParentNode(context, parenNode.child1);
		if (parenNode.child2 != -1) DisplayParentNode(context, parenNode.child2);
		ImGui::TreePop();
	}
	
}

void Update(Context& context, float dt)
{
	auto Graphics = &context.Graphics;
	auto& Renderer3D = context.Renderer3D;

	ControlCameraStateImgui(context.CameraState);
    ControlCamera(context.CameraState, Renderer3D.CurrentCamera, dt);
	
	context.Renderer3D.UpdateCamera();
	
	Graphics->SetBlendingState(BS_AlphaBlending);
	Graphics->ResetRenderTarget();
	Graphics->ClearBuffer(0.0f, 0.0f, 0.0f);
	Graphics->ClearZBuffer();
	Graphics->SetDepthStencilState(DSS_Normal);
	Graphics->SetRasterizationState(RS_NORMAL);

	context.Renderer3D.BeginScene(SC_DEBUG_COLOR);
	context.Renderer3D.DrawDebugGeometry(AXIS, { 0.0f, 0.0f, 0.0f }, float3(1.0f));

	Graphics->SetRasterizationState(RS_DEBUG_NOCULL);
	Graphics->SetDepthStencilState(DSS_2DRendering);

	// rebuilding the vbh

	// rebuilding step by step -- separate list with aabbs that will be inserted into the tree

	// shooting rays
	ImGui::Begin("BVH Visualizer");

	ImGui::Text("Next box bounds:");
	ImGui::DragFloat3("Min Bounds", &context.MinBounds.x, 0.1f , -50.0f, 50.0f);
	ImGui::DragFloat3("Max Bounds", &context.MaxBounds.x, 0.1f , -50.0f, 50.0f);

	ImGui::Checkbox("Show MinMax Bound", &context.ShowBigBBox);

	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);
	if (ImGui::Button("Add Random AABB"))
	{
		for (size_t i = 0; i < context.BoxesToAdd; ++i)
		{
			AABB aabb;
			aabb.Min = float3(RandomFloat(context.MinBounds.x, context.MaxBounds.x),
							  RandomFloat(context.MinBounds.y, context.MaxBounds.y),
							  RandomFloat(context.MinBounds.z, context.MaxBounds.z));

			aabb.Max = float3(RandomFloat(context.MinBounds.x, context.MaxBounds.x),
							  RandomFloat(context.MinBounds.y, context.MaxBounds.y),
							  RandomFloat(context.MinBounds.z, context.MaxBounds.z));

			InsertAABB(context.Bvh, aabb);
		}
	}
	ImGui::SameLine();
	ImGui::InputInt("Num Boxes:", &context.BoxesToAdd, 1, 5);
	ImGui::PopItemWidth();


	ImGui::Separator();

	ImGui::Checkbox("Show All", &context.ShowAllBoxes);
	ImGui::Checkbox("Open all parents", &context.OpenAllParents);
	
	if (context.ShowSelectedLeaf = ImGui::TreeNode("Leaf Nodes"); context.ShowSelectedLeaf)
	{
		TempFormater formater;
		size_t index{0};
		
		for (auto& node : context.Bvh.Nodes)
		{
			++index;
			if (node.child1 != -1 && node.child2 != -1) continue;
			if (ImGui::Selectable(formater.Format("Box {}", index - 1), context.SelectedLeaf == index - 1))
			{
				context.SelectedLeaf = index - 1;
			}
		}

		ImGui::TreePop();
	}

	if (context.ShowSelectedParent = ImGui::TreeNode("Parent Nodes"); context.ShowSelectedParent)
	{
		DisplayParentNode(context, context.Bvh.RootIndex);
		ImGui::TreePop();
	}

	ImGui::End();

	if (context.ShowAllBoxes)
	{
		for (auto& node : context.Bvh.Nodes)
		{
			DrawAABB(Renderer3D, node.box, CUBE_RED);
		}
	}

	if (context.ShowBigBBox) DrawAABB(Renderer3D, {context.MinBounds, context.MaxBounds}, CUBE_GREEN);
	if (context.ShowSelectedLeaf) DrawAABB(Renderer3D, context.Bvh.Nodes[context.SelectedLeaf].box, CUBE_BLUE);
	if (context.ShowSelectedParent)
	{
		auto parent = context.Bvh.Nodes[context.SelectedParent];
		
		if (parent.child1 != -1) DrawAABB(Renderer3D, context.Bvh.Nodes[parent.child1].box, CUBE_RED);
		if (parent.child2 != -1) DrawAABB(Renderer3D, context.Bvh.Nodes[parent.child2].box, CUBE_GREEN);
		
		DrawAABB(Renderer3D, parent.box, CUBE_BLUE);
	}


	Graphics->SetDepthStencilState(DSS_Normal);
	Graphics->SetRasterizationState(RS_NORMAL);	

	
	context.Renderer3D.DrawSkyBox(context.Textures.LoadedCubes[0].Handle);
}
