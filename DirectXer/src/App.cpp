#include "App.hpp"
#include "Glm.hpp"
#include "Input.hpp"
#include "Logging.hpp"
#include "Math.hpp"



static uint32 CUBE;
static uint32 PLANE;
static uint32 LINES;
static uint32 CYLINDER;
static uint32 SPHERE;
static uint32 AXIS;

void App::Init(HWND t_Window)
{
	DXLOG("[RES] Resouces path: {}", Arguments.ResourcesPath.data());

	
	// @Todo: Refactor this into functions
	Graphics.initSwapChain(t_Window, Width, Height);
	Graphics.initBackBuffer();
	Graphics.initZBuffer(Width, Height);
	Graphics.initResources();
	Graphics.initRasterizationsStates();
	Graphics.initSamplers();

	Textures.LoadTextures(Graphics, Arguments.ResourcesPath.data());

	// @Todo: This should some sort of arena storage to do its thing
	BufferBuilder builder;
	CUBE = builder.InitCube(CubeGeometry{}, glm::vec3{1.0f, 0.0f, 0.0f});
	PLANE = builder.InitPlane(PlaneGeometry{}, glm::vec3{0.0f, 1.0f, 0.0f});
	SPHERE = builder.InitSphere(SphereGeometry{}, glm::vec3{0.0f, 1.0f, 0.0f});
	CYLINDER = builder.InitCylinder(CylinderGeometry{0.25, 0.25, 1.5}, glm::vec3{1.0f, 1.0f, 0.0f});
	LINES = builder.InitLines(LinesGeometry{}, glm::vec3{0.8f, 0.8f, 0.8f});
	AXIS = builder.InitAxisHelper();

	GPUGeometry desc = builder.CreateBuffer(Graphics);
	DebugGeometry = desc.Description;
	Graphics.setIndexBuffer(desc.Ibo);
	Graphics.setVertexBuffer(desc.Vbo);

	const char* cube_fils[] = {
		"sky/left.png",
		"sky/right.png",
		"sky/down.png",
		"sky/up.png",
		"sky/front.png",
		"sky/back.png",
	};
	auto cube = Textures.LoadCube(Graphics, Arguments.ResourcesPath, cube_fils);

	
	Graphics.bindTexture(0, CHECKER_TEXTURE.Handle);

	Graphics.bindTexture(1, cube);

	Graphics.setShaderConfiguration(SC_DEBUG_TEX);
	Graphics.setViewport(0, 0, 800, 600);
	Graphics.setRasterizationState(CurrentRastState);

	camera.Pos = {0.0f, 0.0f, -0.5f};
	Graphics.EndFrame();
}

void App::Resize()
{

	Graphics.resizeBackBuffer(Width, Height);
	Graphics.destroyZBuffer();
	Graphics.initZBuffer(Width, Height);
	Graphics.setViewport(0, 0, Width, Height);

}

void App::Spin(float dt)
{
	if(gInput.IsKeyReleased(KeyCode::F11))
	{
		CurrentRastState = RasterizationState{(CurrentRastState + 1) % RS_COUNT};
		Graphics.setRasterizationState(CurrentRastState);
	}
	

	static float t = 0.0;
	t += 1.1 * dt;
	t = t > 100.0f ? 0.0f : t;
	ControlCameraFPS(camera);

	Graphics.ClearBuffer(0.0f, 0.0f, 0.0f);
	Graphics.ClearZBuffer();

	const float ratio =  Width/Height;
	Graphics.m_VertexShaderCB.projection = glm::transpose(glm::perspective(65.0f, ratio, 0.0001f, 1000.0f));
	Graphics.m_VertexShaderCB.view = glm::transpose(camera.view());

	Graphics.setShaderConfiguration(SC_DEBUG_TEX);

	Graphics.setShaderConfiguration(SC_DEBUG_TEX);
	Graphics.m_VertexShaderCB.model = init_rotation(t*0.25f, {0.0f, 1.0f, 0.0f}) * init_translate(0.0f, 1.0f, 0.0f);
	Graphics.updateCBs();
	DebugGeometry.DrawGeometry(Graphics, CUBE);	 

	//Graphics.setShaderConfiguration(SC_DEBUG_COLOR);
	//Graphics.m_VertexShaderCB.model = init_translate(1.0f, 0.0f, 1.0f);
	//Graphics.m_PixelShaderCB.color = {1.0f, 0.0f, 0.0f, 1.0f};
	//Graphics.updateCBs();
	//DebugGeometry.DrawGeometry(Graphics, PLANE);


	Graphics.setShaderConfiguration(SC_DEBUG_SKY);
	Graphics.m_VertexShaderCB.model = init_scale(500.0f, 500.0f, 500.0f) * init_translate(0.0f, 0.0f, 0.0f);
	Graphics.updateCBs();
	DebugGeometry.DrawGeometry(Graphics, CUBE);

	// Graphics.m_VertexShaderCB.model = init_scale(0.5f, 0.5f, 0.5f)*init_translate(-1.5f, 0.0f, 0.0f);
	// Graphics.updateCBs();
	// DebugGeometry.DrawGeometry(Graphics, CYLINDER);

	
	// Graphics.m_VertexShaderCB.model = init_scale(0.5f, 0.5f, 0.5f)*init_translate(0.0f, 1.5f, 0.0f);
	// Graphics.updateCBs();
	// DebugGeometry.DrawGeometry(Graphics, SPHERE);

	// Graphics.m_VertexShaderCB.model = init_translate(0.0f, 0.0f, 0.0f);
	// Graphics.updateCBs();
	// DebugGeometry.DrawGeometry(Graphics, LINES);

	// Graphics.m_VertexShaderCB.model = init_translate(t * 0.01f, 0.0f, 0.0f);
	// Graphics.m_VertexShaderCB.model = init_translate(0.0f, 0.0f, t * 0.01f);
	// Graphics.updateCBs();
	// DebugGeometry.DrawGeometry(Graphics, AXIS);

	Graphics.EndFrame();

}

void App::Destroy()
{
	Graphics.Destroy();
}
