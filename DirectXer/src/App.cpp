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
static uint32 CAMERA;




void App::Init(HWND t_Window)
{
	DXLOG("[RES] Resouces path: {}", Arguments.ResourcesPath.data());

	const float ratio = Width / Height;
	const float pov = 65.0f;
	const float nearPlane = 0.0001f;
	const float farPlane = 1000.0f;

	camera.Pos = {1.0f, 0.5f, 1.0f};
	camera.lookAt({0.0f, 0.0f, 0.0f});

	auto proj = glm::perspective(pov, ratio, nearPlane, farPlane);

	// @Todo: Refactor this into functions
	Graphics.initSwapChain(t_Window, Width, Height);
	Graphics.initBackBuffer();
	Graphics.initZBuffer(Width, Height);
	Graphics.initResources();
	Graphics.initRasterizationsStates();
	Graphics.initSamplers();

	Textures.LoadTextures(Graphics, Arguments.ResourcesPath.data());

	// @Todo: This should use some sort of arena storage to do its thing
	BufferBuilder builder;
	CUBE = builder.InitCube(CubeGeometry{}, glm::vec3{1.0f, 0.0f, 0.0f});
	PLANE = builder.InitPlane(PlaneGeometry{}, glm::vec3{0.0f, 1.0f, 0.0f});
	SPHERE = builder.InitSphere(SphereGeometry{}, glm::vec3{0.0f, 1.0f, 0.0f});
	CYLINDER = builder.InitCylinder(CylinderGeometry{0.25, 0.25, 1.5}, glm::vec3{1.0f, 1.0f, 0.0f});
	LINES = builder.InitLines(LinesGeometry{}, glm::vec3{0.8f, 0.8f, 0.8f});
	AXIS = builder.InitAxisHelper();
	CAMERA = builder.InitCameraHelper({glm::perspective(45.0f, 3.0f/4.0f, 1.50f, 4.0f)});

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
	SkyboxTexture = Textures.LoadCube(Graphics, Arguments.ResourcesPath, cube_fils);


	Graphics.setShaderConfiguration(SC_DEBUG_TEX);
	Graphics.setViewport(0, 0, 800, 600);
	Graphics.setRasterizationState(CurrentRastState);

	Graphics.m_VertexShaderCB.projection = glm::transpose(glm::perspective(pov, ratio, nearPlane, farPlane));
}

void App::Resize()
{
	Graphics.resizeBackBuffer(Width, Height);
	Graphics.destroyZBuffer();
	Graphics.initZBuffer(Width, Height);
	Graphics.setViewport(0, 0, Width, Height);

	const float ratio =  Width/Height;
	const float pov =  65.0f;
	const float nearPlane = 0.0001f;
	const float farPlane = 1000.0f;

	Graphics.m_VertexShaderCB.projection = glm::transpose(glm::perspective(pov, ratio, nearPlane, farPlane));

}

void App::RenderSkyBox()
{
	// @Speed: The texture will be bount most of the time
	Graphics.bindTexture(0, SkyboxTexture);
	Graphics.setShaderConfiguration(SC_DEBUG_SKY);
	Graphics.m_VertexShaderCB.model = init_scale(500.0f, 500.0f, 500.0f) * init_translate(0.0f, 0.0f, 0.0f);
	Graphics.updateCBs();
	DebugGeometry.DrawGeometry(Graphics, CUBE);

}

void App::SetupCamera(Camera t_Camera)
{
	Graphics.m_VertexShaderCB.view = glm::transpose(t_Camera.view());
}

void App::RenderDebugGeometry(uint32 t_Id, glm::mat4 t_Translation, glm::mat4 t_Scale, glm::mat4 t_Rotation)
{
	Graphics.m_VertexShaderCB.model = t_Rotation *  t_Translation * t_Scale;
	Graphics.updateCBs();
	DebugGeometry.DrawGeometry(Graphics, t_Id);
}

void App::RenderDebugGeometryTransform(uint32 t_Id, glm::mat4 t_Transform)
{
	Graphics.m_VertexShaderCB.model = t_Transform;
	Graphics.updateCBs();
	DebugGeometry.DrawGeometry(Graphics, t_Id);
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
	ControlCameraFPS(camera, dt);

	
	// @Note: Rendering begins here
	Graphics.ClearBuffer(0.0f, 0.0f, 0.0f);
	Graphics.ClearZBuffer();

	SetupCamera(camera);


	Graphics.setShaderConfiguration(SC_DEBUG_COLOR);

	RenderDebugGeometry(AXIS, init_translate(0.0f, 0.0f, 0.0f), init_scale(1.0f, 1.0f, 1.0f));
	RenderDebugGeometry(CAMERA, init_translate(2.0f, 3.0f, 2.0f), init_scale(1.0f, 1.0f, 1.0f), init_rotation({0.0, 0.0, 0.0}, {0.0f, 1.0f, 0.0f}));


	
	Graphics.setShaderConfiguration(SC_DEBUG_TEX);

	Graphics.bindTexture(1, CHECKER_TEXTURE.Handle);
	RenderDebugGeometry(CUBE, init_translate(0.0f, 1.0, 4.0f), init_scale(0.25f, 0.25f, 0.25f), init_rotation(t*0.25f, {0.0f, 1.0f, 0.0f}));

	Graphics.bindTexture(1, FLOOR_TEXTURE.Handle);
	RenderDebugGeometry(SPHERE, init_translate(4.0f, std::sin(t*3)*0.5f + 1.5f, 4.0f), init_scale(0.25f, 0.25f, 0.25f));

	Graphics.bindTexture(1, FLOOR_TEXTURE.Handle);
	RenderDebugGeometry(CYLINDER, init_translate(-4.0f, 1.0f, 4.0f), init_scale(0.25f, 0.25f, 0.25f));

	Graphics.bindTexture(1, ROCKS_TEXTURE.Handle);
	RenderDebugGeometry(PLANE, init_translate(0.0f, 0.0, 0.0f), init_scale(3.0f, 1.0f, 3.0f));



	RenderSkyBox();

	Graphics.EndFrame();

}

void App::Destroy()
{
	Graphics.Destroy();
}
