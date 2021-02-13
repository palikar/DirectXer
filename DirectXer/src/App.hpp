#pragma once

#include "Geometry.hpp"
#include "IncludeWin.hpp"
#include "Graphics.hpp"
#include "Camera.hpp"
#include "TextureCatalog.hpp"
#include "GeometryUtils.hpp"
#include "Materials.hpp"

struct CommandLineSettings
{
	std::string_view ResourcesPath;
};

struct Lighting
{
	glm::vec4 ambLightColor{0.0f, 0.0f, 0.0f, 1.0f};

	glm::vec4 dirLightColor{0.0f, 0.0f, 0.0f, 1.0f};
	glm::vec4 dirLightDir{0.5, 0.5f, 0, 0};
};


struct LightSetup
{
	Lighting lighting;
	CBObject bufferId;
};

class App
{
public:

	void Init(HWND t_Window);
	void Spin(float dt);
	void EndFrame();
	void Destroy();
	void Resize();

	void SetupCamera(Camera t_Camera);


	void RenderSkyBox();
	void RenderDebugGeometry(uint32 t_Id, glm::mat4 t_Translation = glm::mat4(1), glm::mat4 t_Scale = glm::mat4(1), glm::mat4 t_Rotation = glm::mat4(1));
	void RenderDebugGeometryTransform(uint32 t_Id, glm::mat4 t_Transform = glm::mat4(1));
	

	TexturedMaterial texMat;
	TexturedMaterialData texMatData;

	PhongMaterial phongMat;
	PhongMaterialData phongMatData;

	
	Graphics Graphics;
	BufferDescriptor DebugGeometry;
	Camera camera;
	RasterizationState CurrentRastState = RS_NORMAL;

	TextureObject SkyboxTexture;

	LightSetup Light;

	float32 Width;
	float32 Height;
	int ReturnValue{0};
	boolean Running{true};
	CommandLineSettings Arguments;
	TextureCatalog Textures;
	
};
