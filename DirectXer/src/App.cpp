#include "App.hpp"
#include "Glm.hpp"
#include "Input.hpp"
#include "Logging.hpp"
#include "Math.hpp"

#include <imgui.h>


static uint32 CUBE;
static uint32 PLANE;
static uint32 LINES;
static uint32 CYLINDER;
static uint32 SPHERE;
static uint32 AXIS;
static uint32 CAMERA;
static uint32 POINTLIGHT;
static uint32 SPOTLIGHT;


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

	const char* cube_fils[] = {
		"sky/left.png",
		"sky/right.png",
		"sky/down.png",
		"sky/up.png",
		"sky/front.png",
		"sky/back.png",
	};
	SkyboxTexture = Textures.LoadCube(Graphics, Arguments.ResourcesPath, cube_fils);


	Renderer2D.InitRenderer(&Graphics, { Width, Height });

	// @Todo: This should use some sort of arena storage to do its thing
	BufferBuilder builder;
	builder.Init(8);
	CUBE = builder.InitCube(CubeGeometry{}, glm::vec3{1.0f, 0.0f, 0.0f});
	PLANE = builder.InitPlane(PlaneGeometry{}, glm::vec3{0.0f, 1.0f, 0.0f});
	SPHERE = builder.InitSphere(SphereGeometry{}, glm::vec3{0.0f, 1.0f, 0.0f});
	CYLINDER = builder.InitCylinder(CylinderGeometry{0.25, 0.25, 1.5}, glm::vec3{1.0f, 1.0f, 0.0f});
	LINES = builder.InitLines(LinesGeometry{}, glm::vec3{0.8f, 0.8f, 0.8f});
	AXIS = builder.InitAxisHelper();
	POINTLIGHT = builder.InitPointLightHelper();
	SPOTLIGHT = builder.InitSpotLightHelper();
	// CAMERA = builder.InitCameraHelper({glm::perspective(45.0f, 3.0f/4.0f, 1.50f, 4.0f)});

	GPUGeometryDesc = builder.CreateBuffer(Graphics);
	DebugGeometry = GPUGeometryDesc.Description;
	Graphics.setIndexBuffer(GPUGeometryDesc.Ibo);
	Graphics.setVertexBuffer(GPUGeometryDesc.Vbo);


	// Create material
	texMat.config = SC_DEBUG_TEX;
	texMat.data = Graphics.createConstantBuffer(sizeof(TexturedMaterialData), &texMatData);
	texMat.BaseMap = ROCKS_TEXTURE.Handle;
	texMat.AoMap = ROCKS_AO_TEXTURE.Handle;
	texMat.EnvMap = SkyboxTexture;

	phongMat.config = SC_DEBUG_PHONG;
	phongMat.data = Graphics.createConstantBuffer(sizeof(PhongMaterialData), &texMatData);


	// Create lighing
	Light.bufferId = Graphics.createConstantBuffer(sizeof(Lighting), &Light.lighting);

	Light.lighting.ambLightColor = {0.7f, 0.7f, 0.7f, 0.4f};
	Light.lighting.dirLightColor = {0.2f, 0.2f, 0.2f, 0.76f};
	Light.lighting.dirLightDir   = {0.5f, 0.471f, 0.0f, 0.0f};

	Light.lighting.pointLights[0].Active = 1;
	
	Graphics.updateCBs(Light.bufferId, sizeof(Lighting), &Light.lighting);

	Graphics.setShaderConfiguration(SC_DEBUG_TEX);
	Graphics.setViewport(0, 0, 800, 600);
	Graphics.setRasterizationState(CurrentRastState);

	camera.Pos = {1.0f, 0.5f, 1.0f};
	camera.lookAt({0.0f, 0.0f, 0.0f});

	Renderer2D.Images.Init(&Graphics, Arguments.ResourcesPath.data());
	ImageLibraryBuilder imagebuilder;
	imagebuilder.PutImage("images/facebook.png");
	imagebuilder.PutImage("images/instagram.png");
	Renderer2D.Images.Build(imagebuilder);

	Graphics.VertexShaderCB.projection = glm::transpose(glm::perspective(pov, Width/Height, nearPlane, farPlane));

}

void App::Resize()
{
	Renderer2D.Params.Width = Width;
	Renderer2D.Params.Height = Height;
	
	Graphics.resizeBackBuffer(Width, Height);
	Graphics.destroyZBuffer();
	Graphics.initZBuffer(Width, Height);
	Graphics.setViewport(0, 0, Width, Height);

	Graphics.VertexShaderCB.projection = glm::transpose(glm::perspective(pov, Width/Height, nearPlane, farPlane));

}

void App::RenderSkyBox()
{
	// @Speed: The texture will be bount most of the time
	Graphics.setShaderConfiguration(SC_DEBUG_SKY);
	Graphics.bindTexture(0, SkyboxTexture);
	Graphics.VertexShaderCB.model = init_scale(500.0f, 500.0f, 500.0f) * init_translate(0.0f, 0.0f, 0.0f);
	Graphics.VertexShaderCB.invModel = glm::inverse(Graphics.VertexShaderCB.model);
	Graphics.updateCBs();
	DebugGeometry.DrawGeometry(Graphics, CUBE);

}

void App::SetupCamera(Camera t_Camera)
{
	Graphics.VertexShaderCB.view = glm::transpose(t_Camera.view());
	Graphics.PixelShaderCB.cameraPos = t_Camera.Pos;
}

void App::RenderDebugGeometry(uint32 t_Id, glm::mat4 t_Translation, glm::mat4 t_Scale, glm::mat4 t_Rotation)
{
	Graphics.VertexShaderCB.model = t_Rotation *  t_Translation * t_Scale;
	Graphics.VertexShaderCB.invModel = glm::inverse(Graphics.VertexShaderCB.model);
	Graphics.updateCBs();
	DebugGeometry.DrawGeometry(Graphics, t_Id);
}

void App::RenderDebugGeometryTransform(uint32 t_Id, glm::mat4 t_Transform)
{
	Graphics.VertexShaderCB.model = t_Transform;
	Graphics.updateCBs();
	DebugGeometry.DrawGeometry(Graphics, t_Id);
}

void App::Spin(float dt)
{

	if(gInput.IsKeyReleased(KeyCode::F1))
	{
		CurrentRastState = RasterizationState((CurrentRastState + 1) % RS_COUNT);
		Graphics.setRasterizationState(CurrentRastState);
	}

	if(gInput.IsKeyReleased(KeyCode::Left))
	{
		CurrentScene = Scene(std::abs(CurrentScene - 1) % SCENE_COUNT);
	}

	if(gInput.IsKeyReleased(KeyCode::Right))
	{
		CurrentScene = Scene((CurrentScene + 1) % SCENE_COUNT);
	}


	switch (CurrentScene)
	{
	  case SCENE_FIRST:
		  ProcessFirstScene(dt);
		  break;
	  case SCENE_PHONGS:
		  ProcessPhongScene(dt);
		  break;
	}
}

void App::EndFrame()
{

}

void App::Destroy()
{
	Graphics.Destroy();
}

void App::ProcessFirstScene(float dt)
{

	static float t = 0.0f;
	t += 1.1f * dt;
	t = t > 100.0f ? 0.0f : t;
	ControlCameraFPS(camera, dt);

	// @Note: Rendering begins here

	Graphics.ClearBuffer(0.0f, 0.0f, 0.0f);
	Graphics.ClearZBuffer();

	Graphics.VertexShaderCB.projection = glm::transpose(glm::perspective(pov, Width/Height, nearPlane, farPlane));
	SetupCamera(camera);

	Graphics.setShaderConfiguration(SC_DEBUG_COLOR);
	Graphics.setIndexBuffer(GPUGeometryDesc.Ibo);
	Graphics.setVertexBuffer(GPUGeometryDesc.Vbo);
	RenderDebugGeometry(AXIS, init_translate(0.0f, 0.0f, 0.0f), init_scale(1.0f, 1.0f, 1.0f));

	Graphics.setShaderConfiguration(SC_DEBUG_TEX);
	Graphics.bindTexture(0, texMat.EnvMap);
	Graphics.bindTexture(1, texMat.BaseMap);
	//Graphics.bindTexture(2, texMat.AoMap);
	Graphics.bindTexture(2, Renderer2D.Images.Atlases[0].TexHandle);

	Graphics.bindPSConstantBuffers(&texMat.data, 1, 1);

	texMatData.Color = { 1.0f, 0.0f, 0.0f, 1.0f };
	texMatData.ColorIntensity = 0.5f * std::abs(std::sin(t*3));
	texMatData.Reflectivity = 0.0f;
	Graphics.updateCBs(texMat.data, sizeof(TexturedMaterialData), &texMatData);
	RenderDebugGeometry(SPHERE, init_translate(4.0f, std::sin(t*3)*0.5f + 1.5f, 4.0f), init_scale(0.25f, 0.25f, 0.25f));

	texMatData.Color = { 1.0f, 0.0f, 1.0f, 1.0f };
	texMatData.ColorIntensity = 0.15;
	Graphics.updateCBs(texMat.data, sizeof(TexturedMaterialData), &texMatData);
	RenderDebugGeometry(CYLINDER, init_translate(-4.0f, 1.0f, 4.0f), init_scale(0.25f, 0.25f, 0.25f));

	texMatData.Color = { 1.0f, 0.0f, 1.0f, 1.0f };
	texMatData.ColorIntensity = 0.0;

	texMatData.Reflectivity = 0.5f;
	Graphics.updateCBs(texMat.data, sizeof(TexturedMaterialData), &texMatData);
	RenderDebugGeometry(PLANE, init_translate(0.0f, 0.0, 0.0f), init_scale(3.0f, 1.0f, 3.0f));


	Graphics.setShaderConfiguration(SC_DEBUG_SIMPLE_TEX);
	Graphics.bindTexture(1, CHECKER_TEXTURE.Handle);
	RenderDebugGeometry(CUBE, init_translate(0.0f, 1.0, 4.0f), init_scale(0.25f, 0.25f, 0.25f), init_rotation(t*0.25f, {0.0f, 1.0f, 0.0f}));

	RenderSkyBox();

	// @Note: UI rendering beggins here
	Renderer2D.BeginScene();
	
	Renderer2D.DrawQuad({10.f, 10.f}, {200.f, 200.f}, {1.0f, 0.0f, 0.0f, 1.0f});
	Renderer2D.DrawQuad({210.f, 210.f}, {50.f, 50.f}, {0.0f, 1.0f, 0.0f, 1.0f}); 
	Renderer2D.DrawQuad({310.f, 310.f}, {20.f, 50.f}, {0.0f, 1.0f, 1.0f, 1.0f});

	Renderer2D.DrawCirlce({510.f, 210.f}, 20.0f, {1.0f, 0.0f, 0.0f, 1.0f});

	Renderer2D.DrawCirlce({210.f, 510.f}, 50.0f, {1.0f, 0.0f, 0.0f, 1.0f});

	Renderer2D.DrawImage(1, {610.0f, 310.0f}, {160.0f, 160.0f});
	
	Renderer2D.EndScene();

}

void App::ProcessPhongScene(float dt)
{
	static float t = 0.0f;
	t += 1.1f * dt;
	t = t > 100.0f ? 0.0f : t;
	ControlCameraFPS(camera, dt);

	bool lightChanged = false;
	Graphics.bindPSConstantBuffers(&Light.bufferId, 1, 2);
	ImGui::Begin("Scene Setup");
	if (ImGui::CollapsingHeader("Ligting"))
	{
		if (ImGui::TreeNode("Directional light"))
		{
			ImGui::Text("Color");
			ImGui::SameLine();
			lightChanged |= ImGui::ColorEdit3("Color:", (float*)&Light.lighting.dirLightColor);
			lightChanged |= ImGui::SliderFloat("Intensity: ", (float*)&Light.lighting.dirLightColor.a, 0.0f, 1.0f, "Amount = %.3f");
			lightChanged |= ImGui::SliderFloat("Angle:", (float*)&Light.lighting.dirLightDir.y, -1.0f, 1.0f, "Direction = %.3f");
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Ambient light"))
		{
			ImGui::Text("Color");
			ImGui::SameLine();
			lightChanged |=ImGui::ColorEdit3("Color", (float*)&Light.lighting.ambLightColor);
			lightChanged |= ImGui::SliderFloat("Intensity: ", (float*)&Light.lighting.ambLightColor.a, 0.0f, 1.0f, "Amount = %.3f");
			ImGui::TreePop();
		}
		
		if (ImGui::TreeNode("Point light"))
		{
			ImGui::Text("Color");
			ImGui::SameLine();
			lightChanged |= ImGui::ColorEdit3("Color", (float*)&Light.lighting.pointLights[0].Color);
			lightChanged |= ImGui::SliderFloat("Constant: ", (float*)&Light.lighting.pointLights[0].Params.r, 0.0f, 2.0f, "Amount = %.3f");
			lightChanged |= ImGui::SliderFloat("Linear: ", (float*)&Light.lighting.pointLights[0].Params.g, 0.0f, 2.0f, "Amount = %.3f");
			lightChanged |= ImGui::SliderFloat("Quadreatic: ", (float*)&Light.lighting.pointLights[0].Params.b, 0.0f, 2.0f, "Amount = %.3f");
			ImGui::TreePop();
		}

		
	}

	
	static float lightRadius = 1.0;
	lightChanged |= ImGui::SliderFloat("Light Radius: ", (float*)&lightRadius, 0.1f, 1.5f, "%.3f");
	float lightX = std::sin(t) * lightRadius;
	float lightY = std::cos(t) * lightRadius;

	Light.lighting.pointLights[0].Position = {lightX, 0.5f, lightY, 0.0f};

	Graphics.updateCBs(Light.bufferId, sizeof(Lighting), &Light.lighting);

	// @Note: Rendering begins here
	Graphics.ClearBuffer(0.0f, 0.0f, 0.0f);
	Graphics.ClearZBuffer();

	SetupCamera(camera);

	Graphics.setIndexBuffer(GPUGeometryDesc.Ibo);
	Graphics.setVertexBuffer(GPUGeometryDesc.Vbo);

	Graphics.setShaderConfiguration(SC_DEBUG_COLOR);
	RenderDebugGeometry(AXIS, init_translate(0.0f, 0.0f, 0.0f), init_scale(1.0f, 1.0f, 1.0f));
	RenderDebugGeometry(SPOTLIGHT, init_translate(2.0f *std::sin(t), 1.0f, 2.0f), init_scale(1.0f, 1.0f, 1.0f));

	Graphics.setRasterizationState(RS_DEBUG);
	RenderDebugGeometry(POINTLIGHT, init_translate(lightX, 0.5f, lightY), init_scale(1.0f, 1.0f, 1.0f));
	Graphics.setRasterizationState(RS_NORMAL);

	Graphics.setShaderConfiguration(SC_DEBUG_PHONG);
	Graphics.bindPSConstantBuffers(&phongMat.data, 1, 3);

	phongMatData.Ambient  = {0.5f, 0.5f, 0.5f, 0.0f };
	phongMatData.Diffuse  = {1.0f, 0.0f, 0.0f, 0.0f };
	phongMatData.Specular = {1.0f, 0.0f, 0.0f, 0.0f };
	phongMatData.Emissive = {0.0f, 0.0f, 0.0f, 0.0f };
	Graphics.updateCBs(phongMat.data, sizeof(PhongMaterialData), &phongMatData);
	RenderDebugGeometry(PLANE, init_translate(0.0f, 0.0, 0.0f), init_scale(5.0f, 1.0f, 5.0f));
	
	phongMatData.Ambient  = {0.5f, 0.5f, 0.5f, 0.0f };
	phongMatData.Diffuse  = {0.0f, 1.0f, 1.0f, 0.0f };
	phongMatData.Specular = {1.0f, 0.0f, 0.0f, 0.0f };
	phongMatData.Emissive = {0.0f, 0.0f, 0.0f, 0.0f };
	phongMatData.SpecularChininess = 0.8f;
	Graphics.updateCBs(phongMat.data, sizeof(PhongMaterialData), &phongMatData);
	RenderDebugGeometry(SPHERE, init_translate(-1.0f, 1.0, 3.0f), init_scale(0.20f, 0.20f, 0.20f));

	phongMatData.Ambient  = {0.5f, 0.5f, 0.5f, 0.0f };
	phongMatData.Diffuse  = {0.0f, 1.0f, 0.0f, 0.0f };
	phongMatData.Specular = {1.0f, 0.0f, 0.0f, 0.0f };
	phongMatData.Emissive = {0.0f, 0.0f, 0.0f, 0.0f };
	phongMatData.SpecularChininess = 1.3f;
	Graphics.updateCBs(phongMat.data, sizeof(PhongMaterialData), &phongMatData);
	RenderDebugGeometry(SPHERE, init_translate(-1.0f, 1.0, -3.0f), init_scale(0.20f, 0.20f, 0.20f));

	ImGui::End();
	RenderSkyBox();
}
