#include <Input.hpp>
#include <Logging.hpp>
#include <Math.hpp>
#include <App.hpp>
#include <Glm.hpp>
#include <Assets.hpp>

#include "ExampleScenes.hpp"
#include <SpaceAssets.hpp>

#include <imgui.h>
#include <ImGuizmo.h>

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
        CUBE = builder.InitCube(CubeGeometry{}, float3{ 1.0f, 0.0f, 0.0f });
        PLANE = builder.InitPlane(PlaneGeometry{}, float3{ 0.0f, 1.0f, 0.0f });
        SPHERE = builder.InitSphere(SphereGeometry{}, float3{ 0.0f, 1.0f, 0.0f });
        CYLINDER = builder.InitCylinder(CylinderGeometry{ 0.25, 0.25, 1.5 }, float3{ 1.0f, 1.0f, 0.0f });
        LINES = builder.InitLines(LinesGeometry{}, float3{ 0.8f, 0.8f, 0.8f });
        AXIS = builder.InitAxisHelper();
        POINTLIGHT = builder.InitPointLightHelper();
        SPOTLIGHT = builder.InitSpotLightHelper();

        Renderer3D.InitDebugGeometry(builder);
    }
    Memory::EndTempScope();

    RocksTextured = 1;
    CheckerTextured = 3;
    FloorTextured = 4;
    SimplePhong = 2;

    // Create materials
    TexturedMaterial rocksMat;
    InitMaterial(Graphics, rocksMat, "RocksdMaterialCB");
    rocksMat.BaseMap = T_ROCKS_COLOR;
    rocksMat.AoMap = T_ROCKS_AO;
    rocksMat.EnvMap = ST_SKY;
    rocksMat.Id = RocksTextured;
    rocksMat.Color = float4(0.0f);

    TexturedMaterial checkerMat;
    InitMaterial(Graphics, checkerMat, "CheckerMaterialCB");
    checkerMat.BaseMap = T_CHECKER;
    checkerMat.AoMap = T_CHECKER;
    checkerMat.EnvMap = ST_SKY;
    checkerMat.Id = CheckerTextured;
    checkerMat.Color = float4(0.0f);

    TexturedMaterial floorMat;
    InitMaterial(Graphics, floorMat, "FlootMaterialCB");
    floorMat.BaseMap = T_FLOOR_COLOR;
    floorMat.AoMap = T_FLOOR_COLOR;;
    floorMat.EnvMap = ST_SKY;
    floorMat.Id = FloorTextured;
    floorMat.Color = float4(0.0f);

    PhongMaterial phongMat;
    phongMat.Ambient  = {0.5f, 0.5f, 0.5f };
    phongMat.Diffuse  = {0.5f, 0.5f, 0.0f };
    phongMat.Specular = {1.0f, 0.0f, 0.0f };
    phongMat.Emissive = {0.0f, 0.0f, 0.0f };
    InitMaterial(Graphics, phongMat, Formater.Format("PhongMaterialCB"));
    phongMat.Id = SimplePhong;

    Renderer3D.MeshData.Materials.TexMaterials.push_back(rocksMat);
    Renderer3D.MeshData.Materials.TexMaterials.push_back(checkerMat);
    Renderer3D.MeshData.Materials.TexMaterials.push_back(floorMat);
    Renderer3D.MeshData.Materials.PhongMaterials.push_back(phongMat);

    Renderer3D.MeshData.Materials.GenerateProxies();

    // Setup the lighting
    Renderer3D.LightingSetup.LightingData.ambLightColor = { 0.7f, 0.7f, 0.7f, 0.4f };
    Renderer3D.LightingSetup.LightingData.dirLightColor = { 0.2f, 0.2f, 0.2f, 0.76f };
    Renderer3D.LightingSetup.LightingData.dirLightDir = { 0.5f, 0.471f, 0.0f, 0.0f };

    Renderer3D.LightingSetup.LightingData.pointLights[0].Active = 0;
    Renderer3D.LightingSetup.LightingData.spotLights[0].Active = 1;

    Renderer3D.LightingSetup.LightingData.spotLights[0].color = {0.5f, 0.5f, 0.5f, 1.0f};
    Renderer3D.LightingSetup.LightingData.spotLights[0].Params = {0.5f, 0.0f, 0.0f, 0.0f};
    Renderer3D.LightingSetup.LightingData.spotLights[0].dir = {0.0f, -0.8f, 0.0f, 0.0f};

    Renderer3D.InitLighting();

    // Setup camera
    Renderer3D.CurrentCamera.Pos = { 1.0f, 0.5f, 1.0f };
    Renderer3D.CurrentCamera.lookAt({ 0.0f, 0.0f, 0.0f });

    Renderer3D.SetupProjection(glm::perspective(pov, Application->Width / Application->Height, nearPlane, farPlane));

    SaveContext = {0};
    SaveContext.Camera = &Renderer3D.CurrentCamera;
    SaveContext.Lighting = &Renderer3D.LightingSetup.LightingData;
    SaveContext.PhongMaterials[0] = &Renderer3D.MeshData.Materials.GetPhong(SimplePhong);

    auto saveFile = Resources::ResolveFilePath("setup.ddata");
    if (PlatformLayer::IsValidPath(saveFile))
    {
        Serialization::LoadFromFile(saveFile, SaveContext);
    }

    SpriteSheets.Init(5, &Renderer2D);
    SpriteSheets.PutSheet(I_SHOOT, { 640.0f, 470.0f }, { 8, 5 });

    uiRenderTarget.Color = NextTextureId();
    uiRenderTarget.DepthStencil = NextTextureId();

    Graphics->SetShaderConfiguration(SC_TEX);
    Graphics->SetViewport(0, 0, 800, 600);
    Graphics->SetRasterizationState(CurrentRastState);

    Graphics->CreateRenderTexture(uiRenderTarget.Color, {(uint16)Application->Width, (uint16)Application->Height, TF_RGBA});
    Graphics->CreateDSTexture(uiRenderTarget.DepthStencil, {(uint16)Application->Width, (uint16)Application->Height, TF_RGBA});

    Renderer3D.InitInstancedDataBuffer(64);
    auto& instData = Renderer3D.AccessInstancedData();
    for (size_t i = 0; i < 32; i++)
    {
        instData[i].model = init_scale(0.5) * init_translate(-4.5f + 1.5f * (8 - (i & 0x0F)), 0.0f, -5.0f + 1.5f * (8 - ((i & 0xF0) >> 4) - 1));
        instData[i].invModel = glm::inverse(instData[i].model);
    }

    for (size_t i = 0; i < 32; i++)
    {
        instData[i + 32].model = init_scale(0.07) * init_translate(-4.5f + 1.5f * (8 - (i & 0x0F)), 3.0f, -5.0f + 1.5f * (8 - ((i & 0xF0) >> 4) - 1));
        instData[i + 32].invModel = glm::inverse(instData[i].model);
    }

    Renderer3D.UpdateInstancedData();

	MeshStore[0] = {M_TREE_1, Material_001, "Tree"};
	MeshStore[1] = {M_SUZANNE, SimpleColor, "Monkey"};

	Meshes.reserve(128);
	MeshesNames.reserve(128);
}

void ExampleScenes::Resize()
{
    Renderer2D.Params.Width = Application->Width;
    Renderer2D.Params.Height = Application->Height;
}

void ExampleScenes::Update(float dt)
{
    if (Input::gInput.IsKeyReleased(KeyCode::F2))
    {
        Application->RenderImGui = !Application->RenderImGui;
    }

    if (Input::gInput.IsKeyReleased(KeyCode::F3))
    {
        Application->EnableVsync = (Application->EnableVsync + 1) % 2;
    }

    if (Input::gInput.IsKeyReleased(KeyCode::Left))
    {
        CurrentScene = Scene(CurrentScene - 1 < 0 ? SCENE_COUNT - 1 : CurrentScene - 1);
    }

    if (Input::gInput.IsKeyReleased(KeyCode::Right))
    {
        CurrentScene = Scene((CurrentScene + 1) % SCENE_COUNT);
    }

    ControlCameraStateImgui(CameraState);
    ControlLightingImGui(Renderer3D.LightingSetup.LightingData);

    if (ImGui::CollapsingHeader("Serialization"))
    {
        if (ImGui::Button("Save Setup"))
        {
            Serialization::DumpToFile(Resources::ResolveFilePath("setup.ddata"), SaveContext);
        }

        ImGui::SameLine();
        if (ImGui::Button("LoadSetup"))
        {
            Serialization::LoadFromFile(Resources::ResolveFilePath("setup.ddata"), SaveContext);
        }
    }

    ImGuiIO &io = ImGui::GetIO();
    ImGuizmo::SetRect(io.DisplayFramebufferScale.x,
                      io.DisplayFramebufferScale.y,
                      io.DisplaySize.x,
                      io.DisplaySize.y);

    switch (CurrentScene)
    {
    case SCENE_FIRST:
        ProcessFirstScene(dt);
        break;
    case SCENE_PHONGS:
        ProcessPhongScene(dt);
        break;
    case SCENE_UI:
        ProcessUIScene(dt);
        break;
    case SCENE_BALLS:
         ProcessBallsScene(dt);
        break;
    case SCENE_OBJECTS:
        ProcessObjectsScene(dt);
        break;
    case SCENE_EDITOR:
        ProcessEditorScene(dt);
        break;
	case SCENE_IMGUI_DEMO:
		ProcessIMGUIScene(dt);
		break;
    }
}

void ExampleScenes::UpdateTime(float dt)
{
    T += 1.0f * dt;
    T = T > 10000.0f ? 0.0f : T;
}

void ExampleScenes::ProcessFirstScene(float dt)
{
    UpdateTime(dt);

    ControlCamera(CameraState, Renderer3D.CurrentCamera, dt);

    Graphics->ClearBuffer(0.0f, 0.0f, 0.0f);
    Graphics->ClearZBuffer();
    Graphics->SetDepthStencilState(DSS_Normal);

    Renderer3D.UpdateDebugData(T);
    Renderer3D.SetupProjection(glm::perspective(pov, Application->Width / Application->Height, nearPlane, farPlane));
    Renderer3D.UpdateCamera();

    // @Note: Rendering begins here
    {

        Renderer3D.BeginScene(SC_DEBUG_COLOR);
        Renderer3D.DrawDebugGeometry(AXIS, { 0.0f, 0.0f, 0.0f }, float3(1.0f));

        {
            Renderer3D.BindMaterial(RocksTextured);

            Renderer3D.DrawDebugGeometry(SPHERE, float3(4.0f, std::sin(T*3)*0.5f + 1.5f, 4.0f), Scale(0.25f));
            Renderer3D.DrawDebugGeometry(CYLINDER, float3(-4.0f, 1.0f, 4.0f), Scale(0.25f));
            Renderer3D.DrawSelectedDebugGeometry(CYLINDER, float3(-4.0f, 1.0f, 4.0f), Scale(0.25f));

            Renderer3D.BindMaterial(CheckerTextured);
            Renderer3D.DrawDebugGeometry(CUBE, float3(0.0f, 1.0, 4.0f), Scale(0.25f), init_rotation(T*0.25f, {0.0f, 1.0f, 0.0f}));

            Renderer3D.BindMaterial(FloorTextured);
            Renderer3D.DrawDebugGeometry(PLANE, float3(0.0f, 0.0, 0.0f), float3(3.0f, 1.0f, 3.0f));
        }

        {
            Renderer3D.BindMaterial(FloorTextured);
            Renderer3D.DrawMesh(M_TREE_1, {0.0f, 1.0f, -4.0f}, Scale(0.05f));
            Renderer3D.DrawSelectedMesh(M_TREE_1, {0.0f, 1.0f, -4.0f}, Scale(0.05f));
        }

        Renderer3D.DrawSkyBox(T_SKY);
    }

	Renderer2D.BeginScene();
    Renderer2D.DrawText("First Scene", {450.0f, 30.0f}, F_DroidSansBold_24, Color::Chartreuse);
    Renderer2D.EndScene();
}

void ExampleScenes::ProcessUIScene(float dt)
{
    Renderer3D.SetupProjection(glm::perspective(pov, Application->Width / Application->Height, nearPlane, farPlane));

    UpdateTime(dt);
    ControlCamera(CameraState, Renderer3D.CurrentCamera, dt);

    Renderer3D.SetupProjection(glm::perspective(pov, Application->Width / Application->Height, nearPlane, farPlane));
    Renderer3D.UpdateCamera();

    Graphics->ClearBuffer(0.0f, 0.0f, 0.0f);
    Graphics->ClearZBuffer();
    Graphics->SetDepthStencilState(DSS_Normal);

    Renderer3D.BeginScene(SC_DEBUG_COLOR);
    Renderer3D.DrawDebugGeometry(AXIS, { 0.0f, 0.0f, 0.0f }, float3(1.0f));
    Renderer3D.DrawSkyBox(T_SKY);

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

	Renderer2D.BeginScene();
    Renderer2D.DrawText("UI Scene", {450.0f, 30.0f}, F_DroidSansBold_24, Color::Chartreuse);
    Renderer2D.EndScene();
}

void ExampleScenes::ProcessPhongScene(float dt)
{
    UpdateTime(dt);
    ControlCamera(CameraState, Renderer3D.CurrentCamera, dt);

    static float lightRadius = 1.0;
    ImGui::SliderFloat("Light Radius: ", (float*)&lightRadius, 0.1f, 1.5f, "%.3f");
    float lightX = std::sin(T) * lightRadius;
    float lightY = std::cos(T) * lightRadius;

    Renderer3D.LightingSetup.LightingData.spotLights[0].position = {2.0f * std::sin(T), 1.0f, 2.0f, 0.0f};
    Renderer3D.LightingSetup.LightingData.pointLights[0].Position = {lightX, 0.5f, lightY, 0.0f};

    Renderer3D.SetupProjection(glm::perspective(pov, Application->Width / Application->Height, nearPlane, farPlane));
    Renderer3D.UpdateLighting();
    Renderer3D.UpdateCamera();
    Renderer3D.BindLighting();

    Graphics->SetDepthStencilState(DSS_Normal);
    Graphics->ClearBuffer(0.0f, 0.0f, 0.0f);
    Graphics->ClearZBuffer();

    Renderer3D.BeginScene(SC_DEBUG_COLOR);
    Renderer3D.DrawDebugGeometry(AXIS, { 0.0f, 0.0f, 0.0f }, float3(1.0f));
    Renderer3D.DrawDebugGeometry(SPOTLIGHT, float3(2.0f *std::sin(T), 1.0f, 2.0f), float3(1.0f, 1.0f, 1.0f));

    Graphics->SetRasterizationState(RS_DEBUG);
    Renderer3D.DrawDebugGeometry(POINTLIGHT, float3(lightX, 0.5f, lightY), float3(1.0f, 1.0f, 1.0f));
    Graphics->SetRasterizationState(RS_NORMAL);

    Renderer3D.BindMaterial(SimplePhong);
    Renderer3D.DrawDebugGeometry(PLANE, float3(0.0f, 0.0, 0.0f), float3(5.0f, 1.0f, 5.0f));
    Renderer3D.DrawDebugGeometry(SPHERE, float3(-1.0f, 1.0, 3.0f), float3(0.20f, 0.20f, 0.20f));
    Renderer3D.DrawDebugGeometry(SPHERE, float3(-1.0f, 1.0, -3.0f), float3(0.20f, 0.20f, 0.20f));

    Renderer3D.DrawSkyBox(T_NIGHT_SKY);

	Renderer2D.BeginScene();
    Renderer2D.DrawText("Phong Scene", {450.0f, 30.0f}, F_DroidSansBold_24, Color::Chartreuse);
    Renderer2D.EndScene();
}

void ExampleScenes::ProcessObjectsScene(float dt)
{
    Renderer3D.SetupProjection(glm::perspective(pov, Application->Width / Application->Height, nearPlane, farPlane));

    UpdateTime(dt);

    Renderer3D.UpdateDebugData(T);
    ControlCamera(CameraState, Renderer3D.CurrentCamera, dt);

    Renderer3D.SetupProjection(glm::perspective(pov, Application->Width / Application->Height, nearPlane, farPlane));
    Renderer3D.UpdateCamera();

    Graphics->ClearBuffer(0.0f, 0.0f, 0.0f);
    Graphics->ClearZBuffer();
    Graphics->SetDepthStencilState(DSS_Normal);

    Renderer3D.BeginScene(SC_DEBUG_COLOR);
    Renderer3D.DrawDebugGeometry(AXIS, { 0.0f, 0.0f, 0.0f }, float3(1.0f));

    Renderer3D.BindLighting();

    Renderer3D.BindMaterial(Material_001);
    Renderer3D.DrawMesh(M_TREE_1, float3{3.0f, -2.0f, 0.0f}, Scale(0.05f));

    Renderer3D.BindMaterial(SimpleColor);

    static mat4 transf = init_translate({-3.0f, 0.0f, 0.0f}) * init_scale(Scale(0.5f));
    ImGuizmo::Enable(true);

    ImGuizmo::AllowAxisFlip(true);

    ImGuizmo::MODE currentGizmoMode(ImGuizmo::WORLD);
    ImGuizmo::OPERATION currentGizmoOperation{ ImGuizmo::TRANSLATE };

    static int op = 0;
    ImGui::Text("Gizmo Operation:");
    ImGui::RadioButton("Translate", &op, 0); ImGui::SameLine();
    ImGui::RadioButton("Rotate", &op, 1); ImGui::SameLine();
    ImGui::RadioButton("Scale", &op, 2);

    if (op == 0) currentGizmoOperation = ImGuizmo::TRANSLATE;
    if (op == 1) currentGizmoOperation = ImGuizmo::ROTATE;
    if (op == 2) currentGizmoOperation = ImGuizmo::SCALE;

    ImGuizmo::BeginFrame();
    transf = glm::transpose(transf);
    ImGuizmo::Manipulate(glm::value_ptr(Renderer3D.CurrentCamera.view()),
                         glm::value_ptr(Renderer3D.CurrentProjection),
                         currentGizmoOperation,
                         currentGizmoMode,
                         glm::value_ptr(transf));
    transf = glm::transpose(transf);

    Renderer3D.DrawMesh(M_SUZANNE, transf);
    Renderer3D.DrawSelectedMesh(M_SUZANNE, transf);

    Renderer3D.BindMaterialInstanced(SimpleColor);
    Renderer3D.DrawInstancedMesh(M_SUZANNE, 32);

    Renderer3D.BindMaterialInstanced(Material_001);
    Renderer3D.DrawInstancedMesh(M_TREE_1, 32, 32);

    Renderer3D.DrawSkyBox(T_SKY);

	Renderer2D.BeginScene();
    Renderer2D.DrawText("Objects Scene", {450.0f, 30.0f}, F_DroidSansBold_24, Color::Chartreuse);
    Renderer2D.EndScene();
}

void ExampleScenes::ProcessBallsScene(float dt)
{
    UpdateTime(dt);

    static int ball_grid_x = 20;
    static int ball_grid_y = 20;
    static float offset = 2.5f;
    static float ballScale = 1.0f;
    static float lightRadius = 1.0;

    if (ImGui::CollapsingHeader("Ball Scene"))
    {
        //ImGui::Text("Balls material");
        //ImGui::ColorEdit3("Ambient Factor:", (float*)&phongMatData.Ambient);
        //ImGui::ColorEdit3("Diffuse Factor:", (float*)&phongMatData.Diffuse);
        //ImGui::ColorEdit3("Specular Factor:", (float*)&phongMatData.Specular);

        ImGui::Separator();
        ImGui::Text("Balls Setup");
        ImGui::SliderInt("Ball Grid X", &ball_grid_x, 10, 30);
        ImGui::SliderInt("Ball Grid Y", &ball_grid_y, 10, 30);
        ImGui::SliderFloat("Offset", &offset, 1.5f, 4.0f);
        ImGui::SliderFloat("Ball Scale", &ballScale, 0.05f, 1.3f);
    }

    Renderer3D.LightingSetup.LightingData.spotLights[0].position = {((ball_grid_x - 3) * offset * 0.5f) * std::sin(T), 1.0f, (0.5f * (ball_grid_y - 3) * offset), 0.0f};
    Renderer3D.LightingSetup.LightingData.pointLights[0].Position = {((ball_grid_x - 3) * offset * 0.5f) * std::sin(T), 1.0f, (- 0.5f * (ball_grid_y - 3) * offset), 0.0f};

    ControlCamera(CameraState, Renderer3D.CurrentCamera, dt);

    Renderer3D.SetupProjection(glm::perspective(pov, Application->Width / Application->Height, nearPlane, farPlane));
    Renderer3D.UpdateCamera();
    Renderer3D.UpdateLighting();

    // The rendering starts here

    Graphics->ClearBuffer(0.0f, 0.0f, 0.0f);
    Graphics->ClearZBuffer();
    Graphics->SetDepthStencilState(DSS_Normal);

    Graphics->SetShaderConfiguration(SC_PHONG);

    for (int i = 0; i < ball_grid_x; ++i)
    {
        for (int j = 0; j < ball_grid_y; ++j)
        {
            float3 ballPosition(i * offset - (ball_grid_x * offset * 0.5f),
                                0.0f,
                                j * offset - (ball_grid_y * offset * 0.5f));

            Renderer3D.DrawDebugGeometry(SPHERE, ballPosition, Scale(ballScale));
        }
    }

    Renderer3D.DrawSkyBox(T_NIGHT_SKY);

	Renderer2D.BeginScene();
    Renderer2D.DrawText("Balls Scene", {450.0f, 30.0f}, F_DroidSansBold_24, Color::Chartreuse);
    Renderer2D.EndScene();
}

void ExampleScenes::ProcessEditorScene(float dt)
{
    UpdateTime(dt);

    Renderer3D.UpdateDebugData(T);
    ControlCamera(CameraState, Renderer3D.CurrentCamera, dt);
    Renderer3D.SetupProjection(glm::perspective(pov, Application->Width / Application->Height, nearPlane, farPlane));
    Renderer3D.UpdateCamera();
    Renderer3D.UpdateLighting();

	Graphics->ClearBuffer(0.0f, 0.0f, 0.0f);
    Graphics->ClearZBuffer();
    Graphics->SetDepthStencilState(DSS_Normal);

    Renderer3D.BeginScene(SC_DEBUG_COLOR);
    Renderer3D.DrawDebugGeometry(AXIS, { 0.0f, 0.0f, 0.0f }, float3(1.0f));

	Renderer3D.BindLighting();
	
	{
		ImGui::Separator();
		if (ImGui::Button("New Object..")) ImGui::OpenPopup("select_new_object");
        if (ImGui::BeginPopup("select_new_object"))
        {
            ImGui::Text("New Object");
            ImGui::Separator();
            for (int i = 0; i < MeshPrototypesCount; i++)
			{
				auto mesh = MeshStore[i];
                if (mesh.Mesh == 0) continue;
                if (ImGui::Selectable(mesh.Name.data()))
				{
                    Meshes.push_back({ mesh.Mesh, mesh.Material, mat4{1}, mesh.Name });
					MeshesNames.push_back(fmt::format("{}_{}", mesh.Name, Meshes.size()));
				}
			}
            ImGui::EndPopup();
        }
		ImGui::Separator();
		

		ImGui::Text("Meshes:");
        {
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
            ImGui::BeginChild("ChildL", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 260), false, window_flags);
			
			for (size_t i = 0; i < Meshes.size(); ++i)
			{
				auto mesh = Meshes[i];
				if (ImGui::Selectable(MeshesNames[i].c_str(), i == SelectedMeshIndex))
				{
					SelectedMesh = mesh.Mesh;
					SelectedMeshIndex = (uint32)i;
				}
			}

			
            ImGui::EndChild();
        }

		for (size_t i = 0; i < Meshes.size(); ++i)
		{
			auto mesh = Meshes[i];
			Renderer3D.BindMaterial(mesh.Material);
			Renderer3D.DrawMesh(mesh.Mesh, mesh.Transform);
		}
		
		if (SelectedMeshIndex < Meshes.size())
		{
			auto& selectedMesh = Meshes[SelectedMeshIndex];

			ImGuizmo::MODE currentGizmoMode(ImGuizmo::WORLD);
			ImGuizmo::OPERATION currentGizmoOperation{ ImGuizmo::TRANSLATE };

			static int op = 0;
			ImGui::Text("Gizmo Operation:");
			ImGui::RadioButton("Translate", &op, 0); ImGui::SameLine();
			ImGui::RadioButton("Rotate", &op, 1); ImGui::SameLine();
			ImGui::RadioButton("Scale", &op, 2);

			if (op == 0) currentGizmoOperation = ImGuizmo::TRANSLATE;
			if (op == 1) currentGizmoOperation = ImGuizmo::ROTATE;
			if (op == 2) currentGizmoOperation = ImGuizmo::SCALE;

			ImGuizmo::BeginFrame();
			selectedMesh.Transform = glm::transpose(selectedMesh.Transform);
			ImGuizmo::Manipulate(glm::value_ptr(Renderer3D.CurrentCamera.view()),
								 glm::value_ptr(Renderer3D.CurrentProjection),
								 currentGizmoOperation,
								 currentGizmoMode,
								 glm::value_ptr(selectedMesh.Transform));
			selectedMesh.Transform = glm::transpose(selectedMesh.Transform);

			Renderer3D.DrawSelectedMesh(selectedMesh.Mesh, selectedMesh.Transform);
		}
		
		
	}
	
	Renderer3D.DrawSkyBox(T_SKY);

	Renderer2D.BeginScene();
    Renderer2D.DrawText("Editor Scene", {450.0f, 30.0f}, F_DroidSansBold_24, Color::Chartreuse);
    Renderer2D.DrawText(Formater.Format("Mesh: {}", MeshesNames[SelectedMeshIndex].c_str()),
						{450.0f, 57.0f}, F_DroidSansBold_24, Color::Chartreuse);
    Renderer2D.EndScene();
}

void ExampleScenes::ProcessIMGUIScene(float dt)
{
	
    UpdateTime(dt);

    Renderer3D.UpdateDebugData(T);
    ControlCamera(CameraState, Renderer3D.CurrentCamera, dt);
    Renderer3D.SetupProjection(glm::perspective(pov, Application->Width / Application->Height, nearPlane, farPlane));
    Renderer3D.UpdateCamera();
    Renderer3D.UpdateLighting();

	Graphics->ClearBuffer(0.0f, 0.0f, 0.0f);
    Graphics->ClearZBuffer();
    Graphics->SetDepthStencilState(DSS_Normal);

    Renderer3D.BeginScene(SC_DEBUG_COLOR);
    Renderer3D.DrawDebugGeometry(AXIS, { 0.0f, 0.0f, 0.0f }, float3(1.0f));

	{
		ImGui::ShowDemoWindow();		
	}
	
	Renderer3D.DrawSkyBox(T_SKY);

	Renderer2D.BeginScene();
    Renderer2D.DrawText("ImGui Scene", {450.0f, 30.0f}, F_DroidSansBold_24, Color::Chartreuse);
    Renderer2D.EndScene();

}
