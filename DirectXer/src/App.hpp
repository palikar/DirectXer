#pragma once

#include "Geometry.hpp"
#include "IncludeWin.hpp"
#include "Graphics.hpp"
#include "Camera.hpp"
#include "TextureCatalog.hpp"
#include "GeometryUtils.hpp"
#include "Materials.hpp"
#include "Lighting.hpp"
#include "2DRendering.hpp"
#include "Memory.hpp"


struct CommandLineSettings
{
	std::string_view ResourcesPath;
};

enum Scene
{
	SCENE_FIRST   = 0,
	SCENE_PHONGS  = 1,
	SCENE_COUNT
};


class App
{
public:

	
	const static inline float pov =  65.0f;
	const static inline float nearPlane = 0.0001f;
	const static inline float farPlane = 1000.0f;


	void Init(HWND t_Window);
	void Spin(float dt);
	void EndFrame();
	void Destroy();
	void Resize();

	void SetupCamera(Camera t_Camera);


	void RenderSkyBox();
	void RenderDebugGeometry(uint32 t_Id, glm::mat4 t_Translation = glm::mat4(1), glm::mat4 t_Scale = glm::mat4(1), glm::mat4 t_Rotation = glm::mat4(1));
	void RenderDebugGeometryTransform(uint32 t_Id, glm::mat4 t_Transform = glm::mat4(1));


	void ProcessFirstScene(float dt);
	void ProcessPhongScene(float dt);
	

	// @Note: Rendering data -- used by the scene to
	// do its rendering
	Graphics Graphics;

	
	TexturedMaterial texMat;
	TexturedMaterialData texMatData;

	PhongMaterial phongMat;
	PhongMaterialData phongMatData;

	BufferDescriptor DebugGeometry;
	Camera camera;
	RasterizationState CurrentRastState;

	TextureObject SkyboxTexture;

	LightSetup Light;

	GPUGeometry GPUGeometryDesc;

	Renderer2D Renderer2D;


	// @Note: Application Data -- used by the "application" for
	// application management stuff
	float32 Width;
	float32 Height;
	int ReturnValue{0};
	boolean Running{true};
	CommandLineSettings Arguments;
	TextureCatalog Textures;

	Scene CurrentScene;
	
};
