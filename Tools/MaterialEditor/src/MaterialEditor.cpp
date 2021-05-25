
#include "Utils.hpp"
#include "MaterialEditor.hpp"

static uint32 CUBE;
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
	context.Textures.LoadedTextures.reserve(64);
	context.Textures.LoadedCubes.reserve(16);

	const char* Envs[] = {
		"resources/night_sky",
		"resources/sky",
	};

	const char* Texs[] = {
		"resources/floor_color.png",
		"resources/rocks_color.png",
		"resources/rocks_normal.png",
		"resources/rocks_ao.png",
		"resources/checker.png",
		"resources/bricks_color.png",
		"resources/bricks_ao.png",
		"resources/bricks_normal.png",
		"resources/dummyTexture.png",
	};

	const char* Meshes[] = {
		"resources/models/first_tree.obj",
		"resources/models/simple_monkey.obj",
	};

	for (size_t i = 0; i < Size(Meshes); i++)
	{
		LoadObjMesh(context, Meshes[i]);
	}

	context.Textures.LoadTextures(Graphics, Texs, (uint32)Size(Texs));
	context.Textures.LoadCubes(Graphics, Envs, (uint32)Size(Envs));

	Memory::EstablishTempScope(Megabytes(4));
	{
		DebugGeometryBuilder builder;
		builder.Init(8);
		CUBE = builder.InitCube(CubeGeometry{}, glm::vec3{ 1.0f, 0.0f, 0.0f });
		PLANE = builder.InitPlane(PlaneGeometry{}, glm::vec3{ 0.0f, 1.0f, 0.0f });
		SPHERE = builder.InitSphere(SphereGeometry{}, glm::vec3{ 0.0f, 1.0f, 0.0f });
		CYLINDER = builder.InitCylinder(CylinderGeometry{ 0.25, 0.25, 1.5 }, glm::vec3{ 1.0f, 1.0f, 0.0f });
		LINES = builder.InitLines(LinesGeometry{}, glm::vec3{ 0.8f, 0.8f, 0.8f });
		AXIS = builder.InitAxisHelper();
		POINTLIGHT = builder.InitPointLightHelper();
		SPOTLIGHT = builder.InitSpotLightHelper();

		context.Renderer3D.InitDebugGeometry(builder);
	}
	Memory::EndTempScope();

	context.Renderer3D.CurrentCamera.Pos = { 1.0f, 0.5f, 1.0f };
	context.Renderer3D.CurrentCamera.lookAt({ 0.0f, 0.0f, 0.0f });
	context.Renderer3D.SetupProjection(glm::perspective(pov, 1080.0f / 720.0f, nearPlane, farPlane));

	context.Renderer3D.MeshData.Materials.Init();

	
	MtlMaterial basicMat = { 0 };
	basicMat.illum = 2;
	basicMat.Ka = {0.2f, 0.2f, 0.2f};
	basicMat.Kd = {0.7f, 0.0f, 0.2f};
	basicMat.Ks = {0.8f, 0.8f, 0.8f};
	basicMat.Id = 1;
	InitMaterial(Graphics, basicMat, "Default Material");

	context.Renderer3D.MeshData.Materials.MtlMaterials.push_back(basicMat);	
	context.Renderer3D.MeshData.Materials.GenerateProxies();

	context.Materials.reserve(32);

	context.Renderer3D.InitLighting();
}

void Update(Context& context, float dt)
{
	auto* currentMesh = &context.Meshes[context.CurrentMeshIndex];
	auto& Renderer3D = context.Renderer3D;
	
	if (ImGui::CollapsingHeader("Mesh Config"))
	{
		ImGui::Combo("Mesh", &context.CurrentMeshIndex, context.MeshNames.data(), (int)context.MeshNames.size());
		currentMesh = &context.Meshes[context.CurrentMeshIndex];
		
		ImGui::SliderFloat("Scale", (float*)&currentMesh->Scale, 0.0f, 5.0f, "%.2f");
		ImGui::SliderFloat3("Position", (float*)&currentMesh->Position, -5.0f, 5.0f, "%.2f");
	}

	if (ImGui::CollapsingHeader("Environment Map"))
	{
		if (ImGui::BeginCombo("Map", context.Textures.LoadedCubes[context.CurrentMapIndex].Name.data()))
		{
			for (int i = 0; i < (int)context.Textures.LoadedCubes.size(); ++i)
			{
				auto& tex = context.Textures.LoadedCubes[i];
				if (ImGui::Selectable(tex.Name.data(), i == context.CurrentMapIndex))
				{
					context.CurrentMapIndex = i;
				}
			}
			ImGui::EndCombo();
		}
	}

	ControlLightingImGui(context.Renderer3D.LightingSetup.LightingData);

	if (ImGui::CollapsingHeader("Ligting Movement"))
	{

		ImGui::Checkbox("Movement", &context.LightHelpersState.Updating);
		ImGui::Separator();
		
		ImGui::Text("Pointlight 1:");
		ImGui::SliderFloat("Radius[1]", &context.LightHelpersState.PointLight1Radius, 0.0f, 10.0f, "%.2f");
		ImGui::SliderFloat("Height[1]", &context.LightHelpersState.PointLight1Height, -5.0f, 10.0f, "%.2f");

		ImGui::Separator();

		ImGui::Text("Pointlight 2:");
		ImGui::SliderFloat("Radius[2]", &context.LightHelpersState.PointLight2Radius, 0.0f, 10.0f, "%.2f");
		ImGui::SliderFloat("Height[2]", &context.LightHelpersState.PointLight2Height, -5.0f, 10.0f, "%.2f");

		ImGui::Separator();

		ImGui::Text("Spotlight:");
		ImGui::SliderFloat("Height[3]", &context.LightHelpersState.SpotHeight, 0.0f, 5.0f, "%.2f");
	}

	ImGui::Begin("Materials");

	ImGui::End();

	if (context.LightHelpersState.Updating) UpdateTime(dt, context.T);
	
	const float lightX1 = std::sin(context.T) * context.LightHelpersState.PointLight1Radius;
	const float lightY1 = std::cos(context.T) * context.LightHelpersState.PointLight1Radius;
	const float lightX2 = std::sin(context.T + PI) * context.LightHelpersState.PointLight2Radius;
	const float lightY2 = std::cos(context.T + PI) * context.LightHelpersState.PointLight2Radius;

	glm::vec3 light1Pos{lightX1, context.LightHelpersState.PointLight1Height, lightY1};
	glm::vec3 light2Pos{lightX2, context.LightHelpersState.PointLight2Height, lightY2};
	glm::vec3 spotLight{ 0.0f, context.LightHelpersState.SpotHeight, 0.0f };
	
	Renderer3D.LightingSetup.LightingData.pointLights[0].Position = glm::vec4(light1Pos, 0.0f);
	Renderer3D.LightingSetup.LightingData.pointLights[1].Position = glm::vec4(light2Pos, 0.0f);

	ControlCameraOrbital(context.Renderer3D.CurrentCamera, dt);
	context.Renderer3D.UpdateCamera();
	
	Renderer3D.UpdateLighting();

	auto Graphics = &context.Graphics;

	Graphics->SetBlendingState(BS_AlphaBlending);
	Graphics->ResetRenderTarget();
	Graphics->ClearBuffer(0.0f, 0.0f, 0.0f);
	Graphics->ClearZBuffer();
	Graphics->SetDepthStencilState(DSS_Normal);
	Graphics->SetRasterizationState(RS_NORMAL);

	context.Renderer3D.BeginScene(SC_DEBUG_COLOR);
	context.Renderer3D.DrawDebugGeometry(AXIS, { 0.0f, 0.0f, 0.0f }, glm::vec3(1.0f));
	Graphics->SetRasterizationState(RS_DEBUG);

	if (Renderer3D.LightingSetup.LightingData.pointLights[0].Active)
		Renderer3D.DrawDebugGeometry(POINTLIGHT, light1Pos, glm::vec3(1.0f));
	
	if (Renderer3D.LightingSetup.LightingData.pointLights[1].Active)
		Renderer3D.DrawDebugGeometry(POINTLIGHT, light2Pos, glm::vec3(1.0f));
	
	Graphics->SetRasterizationState(RS_NORMAL);

	if (Renderer3D.LightingSetup.LightingData.spotLights[0].Active)
		Renderer3D.DrawDebugGeometry(SPOTLIGHT, spotLight, glm::vec3(5.0f));

	context.Renderer3D.MeshData.Materials.Bind(Graphics, 1);

	Renderer3D.EnableLighting();

	context.Renderer3D.DrawMesh(currentMesh->Id, currentMesh->Position, glm::vec3(currentMesh->Scale));
	context.Renderer3D.DrawSkyBox(context.Textures.LoadedCubes[context.CurrentMapIndex].Handle);
}
