#include <Input.hpp>
#include <Logging.hpp>
#include <Math.hpp>
#include <App.hpp>
#include <Glm.hpp>
#include <Assets.hpp>

#include "ExampleScenes.hpp"
#include <SpaceAssets.hpp>

#include <imgui.h>

static uint32 CUBE;
static uint32 PLANE;
static uint32 LINES;
static uint32 CYLINDER; 
static uint32 SPHERE;
static uint32 AXIS;
static uint32 POINTLIGHT;
static uint32 SPOTLIGHT;

static uint32 BGIMAGE = 3;
static uint32 SHIPIMAGE = 4;

void ExampleScenes::Init()
{
	CurrentScene = SCENE_FIRST;
	
	CurrentRastState = RS_NORMAL;

	Renderer2D.InitRenderer(Graphics, { Application->Width, Application->Height });
	//MeshesLib.Init(Graphics);

	Memory::EstablishTempScope(Megabytes(4));
	{
		AssetBuildingContext masterBuilder{0};
		masterBuilder.ImageLib = &Renderer2D.ImageLib;
		masterBuilder.FontLib = &Renderer2D.FontLib;
		masterBuilder.WavLib = &AudioEngine;
		masterBuilder.MeshesLib = &Renderer3D.MeshData;
		masterBuilder.Graphics = Graphics;
	
		AssetStore::LoadAssetFile(AssetFiles[SpaceGameAssetFile], masterBuilder);
		AssetStore::SetDebugNames(Graphics, GPUResources, Size(GPUResources));
	}
	Memory::EndTempScope();

	Renderer3D.InitRenderer(Graphics);

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

		Renderer3D.InitDebugGeometry(builder);
	}
	Memory::EndTempScope();


	// Create material
	texMat.Program = SC_DEBUG_TEX;
	texMat.Cbo = NextConstantBufferId();
	texMat.BaseMap = T_ROCKS_COLOR;
	texMat.AoMap = T_ROCKS_AO;
	texMat.EnvMap = ST_SKY;

	Graphics->CreateConstantBuffer(texMat.Cbo, sizeof(TexturedMaterialData), &texMat);
	Graphics->SetConstantBufferName(texMat.Cbo, "TexMaterialCB");

	phongMat.Program = SC_DEBUG_PHONG;
	phongMat.Cbo = NextConstantBufferId();
	phongMat.Ambient  = {0.5f, 0.5f, 0.5f, 0.0f };
	phongMat.Diffuse  = {0.5f, 0.5f, 0.5f, 0.0f };
	phongMat.Specular = {1.0f, 0.0f, 0.0f, 0.0f };
	phongMat.Emissive = {0.0f, 0.0f, 0.0f, 0.0f };

	Graphics->CreateConstantBuffer(phongMat.Cbo, sizeof(PhongMaterialData), &phongMat);
	Graphics->SetConstantBufferName(phongMat.Cbo, "PhongMaterialCB");

	Renderer3D.Lighting.Lighting.ambLightColor = { 0.7f, 0.7f, 0.7f, 0.4f };
	Renderer3D.Lighting.Lighting.dirLightColor = { 0.2f, 0.2f, 0.2f, 0.76f };
	Renderer3D.Lighting.Lighting.dirLightDir = { 0.5f, 0.471f, 0.0f, 0.0f };

	Renderer3D.Lighting.Lighting.pointLights[0].Active = 0;
	Renderer3D.Lighting.Lighting.spotLights[0].Active = 1;

	Renderer3D.Lighting.Lighting.spotLights[0].color = {0.5f, 0.5f, 0.5f, 1.0f};
	Renderer3D.Lighting.Lighting.spotLights[0].Params = {0.5f, 0.0f, 0.0f, 0.0f};
	Renderer3D.Lighting.Lighting.spotLights[0].dir = {0.0f, -0.8f, 0.0f, 0.0f};

	SaveContext = {0};
	
	SaveContext.Camera = &camera;
	SaveContext.Lighting = &Renderer3D.Lighting.Lighting;
	SaveContext.PhongMaterials[0] = &phongMat;
	
	camera.Pos = { 1.0f, 0.5f, 1.0f };
	camera.lookAt({ 0.0f, 0.0f, 0.0f });

	
	auto saveFile = Resources::ResolveFilePath("setup.ddata");
	if (PlatformLayer::IsValidPath(saveFile))
	{
		Serialization::LoadFromFile(saveFile, SaveContext);
	}	
	
	SpriteSheets.Init(5, &Renderer2D);
	SpriteSheets.PutSheet(I_SHOOT, { 640.0f, 470.0f }, { 8, 5 });

	{
		uiRenderTarget.Color = NextTextureId();
		uiRenderTarget.DepthStencil = NextTextureId();
	
		Graphics->UpdateCBs(Renderer3D.Lighting.Cbo, sizeof(Lighting), &Renderer3D.Lighting.Lighting);

		Graphics->SetShaderConfiguration(SC_DEBUG_TEX);
		Graphics->SetViewport(0, 0, 800, 600);
		Graphics->SetRasterizationState(CurrentRastState);

		Graphics->CreateRenderTexture(uiRenderTarget.Color, {(uint16)Application->Width, (uint16)Application->Height, TF_RGBA});
		Graphics->CreateDSTexture(uiRenderTarget.DepthStencil, {(uint16)Application->Width, (uint16)Application->Height, TF_RGBA});

	}
} 
 
void ExampleScenes::Resize()
{
	Renderer2D.Params.Width = Application->Width;
	Renderer2D.Params.Height = Application->Height;
}

void ExampleScenes::Update(float dt)
{
	if (Input::gInput.IsKeyReleased(KeyCode::F1))
	{
		CurrentRastState = RasterizationState((CurrentRastState + 1) % RS_COUNT);
		Graphics->SetRasterizationState(CurrentRastState);
	}

	if (Input::gInput.IsKeyReleased(KeyCode::Left))
	{
		CurrentScene = Scene(CurrentScene - 1 < 0 ? SCENE_COUNT - 1 : CurrentScene - 1);
	}

	if (Input::gInput.IsKeyReleased(KeyCode::Right))
	{
		CurrentScene = Scene((CurrentScene + 1) % SCENE_COUNT);
	}


	switch (CurrentScene)
	{
	case SCENE_FIRST:
		ProcessFirstScene(dt);
		break;
	case SCENE_PHONGS:
		// ProcessPhongScene(dt);
		break;
	case SCENE_SPACE_GAME:
		// ProcessSpaceScene(dt);
		break;
	case SCENE_OBJECTS:
		// ProcessObjectsScene(dt);
		break;
	case SCENE_BALLS:
		// ProcessBallsScene(dt);
		break;
		
	}

}

void ExampleScenes::ProcessFirstScene(float dt)
{

	static float t = 0.0f;
	t += 1.0f * dt;
	t = t > 100.0f ? 0.0f : t;
	ControlCameraFPS(camera, dt);

	// @Note: Rendering begins here

	Graphics->ClearBuffer(0.0f, 0.0f, 0.0f);
	Graphics->ClearZBuffer();
	Graphics->SetDepthStencilState(DSS_Normal);

	Renderer3D.SetupProjection(glm::perspective(pov, Application->Width/ Application->Height, nearPlane, farPlane));
	Renderer3D.SetupCamera(camera);

	Renderer3D.BeginScene(SC_DEBUG_COLOR);

	Renderer3D.DrawDebugGeometry(AXIS, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f });

	Graphics->SetShaderConfiguration(SC_DEBUG_TEX);
	Graphics->BindTexture(0, texMat.EnvMap);
	Graphics->BindTexture(1, texMat.BaseMap);
	Graphics->BindTexture(2, texMat.AoMap);

	Graphics->BindPSConstantBuffers(texMat.Cbo, 1);

	texMat.Color = { 1.0f, 0.0f, 0.0f, 1.0f };
	texMat.ColorIntensity = 0.5f * std::abs(std::sin(t*3));
	texMat.Reflectivity = 0.0f;
	Graphics->UpdateCBs(texMat.Cbo, sizeof(TexturedMaterialData), &texMat);
	RenderDebugGeometry(SPHERE, init_translate(4.0f, std::sin(t*3)*0.5f + 1.5f, 4.0f), init_scale(0.25f, 0.25f, 0.25f));

	texMat.Color = { 1.0f, 0.0f, 1.0f, 1.0f };
	texMat.ColorIntensity = 0.15f;
	Graphics->UpdateCBs(texMat.Cbo, sizeof(TexturedMaterialData), &texMat);
	RenderDebugGeometry(CYLINDER, init_translate(-4.0f, 1.0f, 4.0f), init_scale(0.25f, 0.25f, 0.25f));

	texMat.Color = { 1.0f, 0.0f, 1.0f, 1.0f };
	texMat.ColorIntensity = 0.0;

	texMat.Reflectivity = 0.5f;
	Graphics->UpdateCBs(texMat.Cbo, sizeof(TexturedMaterialData), &texMat);
	RenderDebugGeometry(PLANE, init_translate(0.0f, 0.0, 0.0f), init_scale(3.0f, 1.0f, 3.0f));


	Graphics->SetShaderConfiguration(SC_DEBUG_SIMPLE_TEX);
	Graphics->BindTexture(1, T_CHECKER);
	RenderDebugGeometry(CUBE, init_translate(0.0f, 1.0, 4.0f), init_scale(0.25f, 0.25f, 0.25f), init_rotation(t*0.25f, {0.0f, 1.0f, 0.0f}));

	Renderer3D.DrawMesh(M_TREE_1, {0.0f, 1.0f, -4.0f}, {0.05f, 0.05f, 0.05f});


	Renderer3D.DrawSkyBox(T_SKY);

	// @Note: UI rendering beggins here
	Renderer2D.BeginScene();
	
	Renderer2D.DrawQuad({10.f, 10.f}, {200.f, 200.f}, {1.0f, 0.0f, 0.0f, 1.0f});
	Renderer2D.DrawQuad({210.f, 210.f}, {50.f, 50.f}, {0.0f, 1.0f, 0.0f, 1.0f}); 
	Renderer2D.DrawQuad({310.f, 310.f}, {20.f, 50.f}, {0.0f, 1.0f, 1.0f, 1.0f});
	Renderer2D.DrawCirlce({510.f, 210.f}, 20.0f, {1.0f, 0.0f, 0.0f, 1.0f});
	Renderer2D.DrawCirlce({210.f, 510.f}, 50.0f, {1.0f, 0.0f, 0.0f, 1.0f});
	Renderer2D.DrawRoundedQuad({610.0f, 110.0f}, {150.f, 150.f}, {0.0f, 1.0f, 1.0f, 1.0f}, 10.0f);

	Renderer2D.DrawImage(I_INSTAGRAM, {610.0f, 310.0f}, {64.0f, 64.0f});

	Renderer2D.DrawText("Hello, Sailor", {400.0f, 400.0f}, F_DroidSansBold_24);
	Renderer2D.DrawText("Hello, Sailor", {400.0f, 435.0f}, F_DroidSans_24);

	glm::vec2 triangle[] = {
		{500.0f, 500.0f},
		{500.0f, 550.0f},
		{530.0f, 525.0f},
	};
	Renderer2D.DrawTriangle(triangle, {1.0f, 0.5f, 1.0f, 1.0f});

	glm::vec2 polygon[] = {
		{560.0f, 560.0f},
		{590.0f, 530.0f},
		{600.0f, 560.0f},
		{555.0f, 590.0f},
	};
	Renderer2D.DrawFourPolygon(polygon, Color::AquaMarine);
	
	static uint32 spriteIndex = 0;
	static float acc = 0;
	acc += dt * 0.3f;
	if (acc > 1.0f/24.0f)
	{
		spriteIndex = spriteIndex + 1 >= 7 ? 0 : ++spriteIndex;
		acc = 0.0f;
	}
	SpriteSheets.DrawSprite(0, spriteIndex, {400.0f, 480.0f}, {64.0f, 64.0f});

	Renderer2D.EndScene();


	Renderer2D.BeginScene(TT_LINES);

	Renderer2D.DrawLine({600.0f, 300.0f}, {620.0f, 400.0f}, {0.5f, 0.5f, 1.0f, 1.0f});
	
	Renderer2D.EndScene();
}

// void ExampleScenes::ProcessPhongScene(float dt)
// {
// 	static float t = 0.0f;
// 	t += 1.1f * dt;
// 	t = t > 100.0f ? 0.0f : t;
// 	ControlCameraFPS(camera, dt);

// 	bool lightChanged = false;
// 	Graphics->BindPSConstantBuffers(Light.bufferId, 2);
	
// 	if (ImGui::CollapsingHeader("Ligting"))
// 	{
// 		if (ImGui::TreeNode("Directional light"))
// 		{
// 			ImGui::Text("Color");
// 			ImGui::SameLine();
// 			lightChanged |= ImGui::ColorEdit3("Color:", (float*)&Light.lighting.dirLightColor);
// 			lightChanged |= ImGui::SliderFloat("Intensity: ", (float*)&Light.lighting.dirLightColor.a, 0.0f, 1.0f, "Amount = %.3f");
// 			lightChanged |= ImGui::SliderFloat("Angle:", (float*)&Light.lighting.dirLightDir.y, -1.0f, 1.0f, "Direction = %.3f");
// 			ImGui::TreePop();
// 		}

// 		if (ImGui::TreeNode("Ambient light"))
// 		{
// 			ImGui::Text("Color");
// 			ImGui::SameLine();
// 			lightChanged |=ImGui::ColorEdit3("Color", (float*)&Light.lighting.ambLightColor);
// 			lightChanged |= ImGui::SliderFloat("Intensity: ", (float*)&Light.lighting.ambLightColor.a, 0.0f, 1.0f, "Amount = %.3f");
// 			ImGui::TreePop();
// 		}
		
// 		if (ImGui::TreeNode("Point light"))
// 		{
// 			ImGui::Checkbox("Active", (bool*)&Light.lighting.pointLights[0].Active);
			
// 			ImGui::Text("Color");
// 			ImGui::SameLine();
// 			lightChanged |= ImGui::ColorEdit3("Color", (float*)&Light.lighting.pointLights[0].Color);
// 			lightChanged |= ImGui::SliderFloat("Constant: ", (float*)&Light.lighting.pointLights[0].Params.r, 0.0f, 2.0f, "Amount = %.3f");
// 			lightChanged |= ImGui::SliderFloat("Linear: ", (float*)&Light.lighting.pointLights[0].Params.g, 0.0f, 2.0f, "Amount = %.3f");
// 			lightChanged |= ImGui::SliderFloat("Quadreatic: ", (float*)&Light.lighting.pointLights[0].Params.b, 0.0f, 2.0f, "Amount = %.3f");
// 			ImGui::TreePop();
// 		}

// 		if (ImGui::TreeNode("Spot Light"))
// 		{
// 			ImGui::Checkbox("Active", (bool*)&Light.lighting.spotLights[0].Active);
			
// 			ImGui::Text("Color");
// 			ImGui::SameLine();
// 			lightChanged |= ImGui::ColorEdit3("Color", (float*)&Light.lighting.spotLights[0].color);
// 			lightChanged |= ImGui::SliderFloat("Intensity", (float*)&Light.lighting.spotLights[0].color.a, 0.0f, 1.0f, "%.3f");
			
// 			lightChanged |= ImGui::SliderFloat("Outer Rad", (float*)&Light.lighting.spotLights[0].Params.r, 0.0f, 5.0f, "%.3f");
// 			lightChanged |= ImGui::SliderFloat("Inner Rad", (float*)&Light.lighting.spotLights[0].Params.g, 0.0f, 5.0f, "%.3f");
// 			ImGui::TreePop();
// 		}		
// 	}
	
// 	static float lightRadius = 1.0;
// 	lightChanged |= ImGui::SliderFloat("Light Radius: ", (float*)&lightRadius, 0.1f, 1.5f, "%.3f");
// 	float lightX = std::sin(t) * lightRadius;
// 	float lightY = std::cos(t) * lightRadius;

// 	Light.lighting.spotLights[0].position = {2.0f *std::sin(t), 1.0f, 2.0f, 0.0f};
	
// 	Light.lighting.pointLights[0].Position = {lightX, 0.5f, lightY, 0.0f};

// 	Graphics->UpdateCBs(Light.bufferId, sizeof(Lighting), &Light.lighting);

// 	// @Note: Rendering begins here
// 	Graphics->SetDepthStencilState(DSS_Normal);
// 	Graphics->ClearBuffer(0.0f, 0.0f, 0.0f);
// 	Graphics->ClearZBuffer();

// 	Graphics->VertexShaderCB.projection = glm::transpose(glm::perspective(pov, Application->Width/Application->Height, nearPlane, farPlane));
// 	SetupCamera(camera);

// 	Graphics->BindIndexBuffer(GPUGeometryDesc.Ibo);
// 	Graphics->BindVertexBuffer(GPUGeometryDesc.Vbo);

// 	Graphics->SetShaderConfiguration(SC_DEBUG_COLOR);
// 	RenderDebugGeometry(AXIS, init_translate(0.0f, 0.0f, 0.0f), init_scale(1.0f, 1.0f, 1.0f));
// 	RenderDebugGeometry(SPOTLIGHT, init_translate(2.0f *std::sin(t), 1.0f, 2.0f), init_scale(1.0f, 1.0f, 1.0f));

// 	Graphics->SetRasterizationState(RS_DEBUG);
// 	RenderDebugGeometry(POINTLIGHT, init_translate(lightX, 0.5f, lightY), init_scale(1.0f, 1.0f, 1.0f));
// 	Graphics->SetRasterizationState(RS_NORMAL);

// 	Graphics->SetShaderConfiguration(SC_DEBUG_PHONG);
// 	Graphics->BindPSConstantBuffers(phongMat.data, 3);

// 	phongMatData.Ambient  = {0.5f, 0.5f, 0.5f, 0.0f };
// 	phongMatData.Diffuse  = {1.0f, 0.0f, 0.0f, 0.0f };
// 	phongMatData.Specular = {1.0f, 0.0f, 0.0f, 0.0f };
// 	phongMatData.Emissive = {0.0f, 0.0f, 0.0f, 0.0f };
// 	Graphics->UpdateCBs(phongMat.data, sizeof(PhongMaterialData), &phongMatData);
// 	RenderDebugGeometry(PLANE, init_translate(0.0f, 0.0, 0.0f), init_scale(5.0f, 1.0f, 5.0f));
	
// 	phongMatData.Ambient  = {0.5f, 0.5f, 0.5f, 0.0f };
// 	phongMatData.Diffuse  = {0.0f, 1.0f, 1.0f, 0.0f };
// 	phongMatData.Specular = {1.0f, 0.0f, 0.0f, 0.0f };
// 	phongMatData.Emissive = {0.0f, 0.0f, 0.0f, 0.0f };
// 	phongMatData.SpecularChininess = 0.8f;
// 	Graphics->UpdateCBs(phongMat.data, sizeof(PhongMaterialData), &phongMatData);
// 	RenderDebugGeometry(SPHERE, init_translate(-1.0f, 1.0, 3.0f), init_scale(0.20f, 0.20f, 0.20f));

// 	phongMatData.Ambient  = {0.5f, 0.5f, 0.5f, 0.0f };
// 	phongMatData.Diffuse  = {0.0f, 1.0f, 0.0f, 0.0f };
// 	phongMatData.Specular = {1.0f, 0.0f, 0.0f, 0.0f };
// 	phongMatData.Emissive = {0.0f, 0.0f, 0.0f, 0.0f };
// 	phongMatData.SpecularChininess = 1.3f;
// 	Graphics->UpdateCBs(phongMat.data, sizeof(PhongMaterialData), &phongMatData);
// 	RenderDebugGeometry(SPHERE, init_translate(-1.0f, 1.0, -3.0f), init_scale(0.20f, 0.20f, 0.20f));

// 	RenderSkyBox();
// }

// void ExampleScenes::ProcessSpaceScene(float dt)
// {
// 	// @Note: UI

// 	// Graphics->SetRenderTarget(uiRenderTarget);
// 	// Graphics->ClearRT(uiRenderTarget);

// 	// Renderer2D.BeginScene();
// 	// Renderer2D.EndScene();

// 	Graphics->ResetRenderTarget();


// 	// @Note: Main Scene

// 	Graphics->ClearBuffer(0.0f, 0.0f, 0.0f);
// 	Graphics->ClearZBuffer();
	

// 	// @Note: Draw the background as the last thing so that the least amount of framgents can get processed
// 	Graphics->SetDepthStencilState(DSS_2DRendering);
// 	Renderer2D.BeginScene();
// 	Renderer2D.DrawImage(BGIMAGE, {0.0f, 0.0f}, {Application->Width, Application->Height});
// 	Renderer2D.EndScene();

// 	Renderer2D.BeginScene();
// 	Renderer2D.DrawImage(SHIPIMAGE, {300.0f, Application->Height - 100.0f}, {64.0f, 64.0f});
// 	Renderer2D.EndScene();
	
	
// }

// void ExampleScenes::ProcessObjectsScene(float dt)
// {
// 	static float t = 0.0f;
// 	t += 1.0f * dt;
// 	t = t > 100.0f ? 0.0f : t;
	
// 	bool lightChanged = false;
// 	Graphics->BindPSConstantBuffers(Light.bufferId, 2);
// 	if (ImGui::CollapsingHeader("Ligting"))
// 	{
// 		if (ImGui::TreeNode("Directional light"))
// 		{
// 			ImGui::Text("Color");
// 			ImGui::SameLine();

// 			lightChanged |= ImGui::ColorEdit3("Color:", (float*)&Light.lighting.dirLightColor);
// 			lightChanged |= ImGui::SliderFloat("Intensity: ", (float*)&Light.lighting.dirLightColor.a, 0.0f, 1.0f, "Amount = %.3f");
// 			lightChanged |= ImGui::SliderFloat("Angle:", (float*)&Light.lighting.dirLightDir.y, -1.0f, 1.0f, "Direction = %.3f");
// 			ImGui::TreePop();
// 		}

// 		if (ImGui::TreeNode("Ambient light"))
// 		{
// 			ImGui::Text("Color");
// 			ImGui::SameLine();
// 			lightChanged |=ImGui::ColorEdit3("Color", (float*)&Light.lighting.ambLightColor);
// 			lightChanged |= ImGui::SliderFloat("Intensity: ", (float*)&Light.lighting.ambLightColor.a, 0.0f, 1.0f, "Amount = %.3f");
// 			ImGui::TreePop();
// 		}
		
// 		if (ImGui::TreeNode("Point light"))
// 		{
// 			ImGui::Text("Color");
// 			ImGui::SameLine();
// 			lightChanged |= ImGui::ColorEdit3("Color", (float*)&Light.lighting.pointLights[0].Color);
// 			lightChanged |= ImGui::SliderFloat("Constant: ", (float*)&Light.lighting.pointLights[0].Params.r, 0.0f, 2.0f, "Amount = %.3f");
// 			lightChanged |= ImGui::SliderFloat("Linear: ", (float*)&Light.lighting.pointLights[0].Params.g, 0.0f, 2.0f, "Amount = %.3f");
// 			lightChanged |= ImGui::SliderFloat("Quadreatic: ", (float*)&Light.lighting.pointLights[0].Params.b, 0.0f, 2.0f, "Amount = %.3f");
// 			ImGui::TreePop();
// 		}		
// 	}

// 	static float lightRadius = 1.0;
// 	lightChanged |= ImGui::SliderFloat("Light Radius: ", (float*)&lightRadius, 0.1f, 1.5f, "%.3f");
// 	float lightX = std::sin(t) * lightRadius;
// 	float lightY = std::cos(t) * lightRadius;

// 	Light.lighting.pointLights[0].Position = {lightX, 0.5f, lightY, 0.0f};

// 	Graphics->UpdateCBs(Light.bufferId, sizeof(Lighting), &Light.lighting);
// 	ControlCameraFPS(camera, dt);

// 	// @Note: Rendering begins here

// 	Graphics->ClearBuffer(0.0f, 0.0f, 0.0f);
// 	Graphics->ClearZBuffer();
// 	Graphics->SetDepthStencilState(DSS_Normal);

// 	Graphics->VertexShaderCB.projection = glm::transpose(glm::perspective(pov, Application->Width/ Application->Height, nearPlane, farPlane));
// 	SetupCamera(camera);

// 	Graphics->SetShaderConfiguration(SC_DEBUG_PHONG);
// 	Graphics->BindPSConstantBuffers(phongMat.data, 3);

// 	phongMatData.Ambient  = {0.5f, 0.5f, 0.5f, 0.0f };
// 	phongMatData.Diffuse  = {1.0f, 0.0f, 0.0f, 0.0f };
// 	phongMatData.Specular = {1.0f, 0.0f, 0.0f, 0.0f };
// 	phongMatData.Emissive = {0.0f, 0.0f, 0.0f, 0.0f };
// 	Graphics->UpdateCBs(phongMat.data, sizeof(PhongMaterialData), &phongMatData);
// 	// RenderDebugGeometry(PLANE, init_translate(0.0f, 0.0, 0.0f), init_scale(5.0f, 1.0f, 5.0f));

// 	/*MeshesLib.DrawMesh(M_TREE_1, {0.0f, 3.0f, 0.0f}, {0.5f, 0.5f, 0.5f}, Light.bufferId);
// 	MeshesLib.DrawMesh(M_SUZANNE, {0.0f, -3.0f, 0.0f}, {4.05f, 4.05f, 4.05f}, Light.bufferId);

// 	{
// 		Graphics->SetShaderConfiguration(SC_MTL_2_INSTANCED);
// 		auto mesh = MeshesLib.Meshes.at(M_SUZANNE);

// 		Graphics->BindIndexBuffer(mesh.Geometry.IndexBuffer);
// 		Graphics->BindVertexBuffer(mesh.Geometry.VertexBuffer, 0, 0);
// 		Graphics->BindVertexBuffer(InstDataBuffer, 0, 1);

// 		Graphics->UpdateCBs();
	
// 		Graphics->DrawInstancedIndex(TT_TRIANGLES, mesh.Geometry.IndexCount, 32, 0, 0);

// 	}*/

// 	RenderSkyBox(T_NIGHT_SKY);
	
// }

// void ExampleScenes::ProcessBallsScene(float dt)
// {
// 	static float t = 0.0f;
// 	t += 1.1f * dt;
// 	t = t > 100.0f ? 0.0f : t;
	
// 	static int ball_grid_x = 20;
//     static int ball_grid_y = 20;
// 	static float offset = 2.5f;
// 	static float ballScale = 1.0f;
// 	static float lightRadius = 1.0;

// 	ControlLightingImGui(Light.lighting);
	
// 	if (ImGui::CollapsingHeader("Ball Scene"))
// 	{
// 		ImGui::Text("Balls material");
// 		ImGui::ColorEdit3("Ambient Factor:", (float*)&phongMatData.Ambient);
// 		ImGui::ColorEdit3("Diffuse Factor:", (float*)&phongMatData.Diffuse);
// 		ImGui::ColorEdit3("Specular Factor:", (float*)&phongMatData.Specular);

// 		ImGui::Separator();
// 		ImGui::Text("Balls Setup");
// 		ImGui::SliderInt("Ball Grid X", &ball_grid_x, 10, 30);
// 		ImGui::SliderInt("Ball Grid Y", &ball_grid_y, 10, 30);
// 		ImGui::SliderFloat("Offset", &offset, 1.5f, 4.0f);
// 		ImGui::SliderFloat("Ball Scale", &ballScale, 0.05f, 1.3f);
// 	}

// 	if (ImGui::CollapsingHeader("Serialization"))
// 	{
// 		if (ImGui::Button("Save Setup"))
// 		{
// 			Serialization::DumpToFile(Resources::ResolveFilePath("setup.ddata"), SaveContext);
// 		}

// 		ImGui::SameLine();
// 		if (ImGui::Button("LoadSetup"))
// 		{
// 			Serialization::LoadFromFile(Resources::ResolveFilePath("setup.ddata"), SaveContext);
// 		}
// 	}

	
// 	ControlCameraFPS(camera, dt);
	
// 	Light.lighting.spotLights[0].position = {((ball_grid_x - 3) * offset * 0.5f) * std::sin(t), 1.0f, (0.5f * (ball_grid_y - 3) * offset), 0.0f};
// 	Light.lighting.pointLights[0].Position = {((ball_grid_x - 3) * offset * 0.5f) * std::sin(t), 1.0f, (- 0.5f * (ball_grid_y - 3) * offset), 0.0f};

// 	ControlCameraFPS(camera, dt);

// 	// The rendering starts here
	
// 	Graphics->ClearBuffer(0.0f, 0.0f, 0.0f);
// 	Graphics->ClearZBuffer();
// 	Graphics->SetDepthStencilState(DSS_Normal);

// 	Graphics->SetShaderConfiguration(SC_DEBUG_PHONG);
	
// 	Graphics->BindIndexBuffer(GPUGeometryDesc.Ibo);
// 	Graphics->BindVertexBuffer(GPUGeometryDesc.Vbo);

// 	Graphics->VertexShaderCB.projection = glm::transpose(glm::perspective(pov, Application->Width/ Application->Height, nearPlane, farPlane));
// 	SetupCamera(camera);

	
// 	Graphics->BindPSConstantBuffers(texMat.data, 1);
// 	Graphics->BindPSConstantBuffers(Light.bufferId, 2);
// 	Graphics->BindPSConstantBuffers(phongMat.data, 3);

// 	Graphics->UpdateCBs(Light.bufferId, sizeof(Lighting), &Light.lighting);
// 	Graphics->UpdateCBs(phongMat.data, sizeof(PhongMaterialData), &phongMatData);
// 	Graphics->UpdateCBs();

// 	for (int i = 0; i < ball_grid_x; ++i)
// 	{
// 		for (int j = 0; j < ball_grid_y; ++j)
// 		{
// 			glm::vec3 ballPosition(i * offset - (ball_grid_x * offset * 0.5f),
// 								   0.0f,
// 								   j * offset - (ball_grid_y * offset * 0.5f));

// 			RenderDebugGeometry(SPHERE, init_translate(ballPosition), init_scale(ballScale, ballScale, ballScale));
// 		}

// 	}

// 	RenderSkyBox(T_NIGHT_SKY);
// }
