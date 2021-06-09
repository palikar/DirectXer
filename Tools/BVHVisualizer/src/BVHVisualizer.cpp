
#include "BVHVisualizer.hpp"

namespace fs = std::filesystem;

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
	
}

void Update(Context& context, float dt)
{
	auto Graphics = &context.Graphics;
	auto& Renderer3D = context.Renderer3D;

	ControlCameraOrbital(context.CameraState, context.Renderer3D.CurrentCamera, dt);
	context.Renderer3D.UpdateCamera();
	
	Graphics->SetBlendingState(BS_AlphaBlending);
	Graphics->ResetRenderTarget();
	Graphics->ClearBuffer(0.0f, 0.0f, 0.0f);
	Graphics->ClearZBuffer();
	Graphics->SetDepthStencilState(DSS_Normal);
	Graphics->SetRasterizationState(RS_NORMAL);

	context.Renderer3D.BeginScene(SC_DEBUG_COLOR);
	context.Renderer3D.DrawDebugGeometry(AXIS, { 0.0f, 0.0f, 0.0f }, glm::vec3(1.0f));

	Graphics->SetRasterizationState(RS_DEBUG);
	Graphics->SetDepthStencilState(DSS_2DRendering);
	
	context.Renderer3D.DrawDebugGeometry(CUBE, { 0.0f, 0.0f, 0.0f }, Scale(3.0f));
	
	Graphics->SetDepthStencilState(DSS_Normal);
	Graphics->SetRasterizationState(RS_NORMAL);
	

	
	context.Renderer3D.DrawSkyBox(context.Textures.LoadedCubes[0].Handle);
}
