#pragma once

#include <Geometry.hpp>
#include <Graphics.hpp>
#include <Camera.hpp>
#include <TextureCatalog.hpp>
#include <GeometryUtils.hpp>
#include <Materials.hpp>
#include <Lighting.hpp>
#include <2DRendering.hpp>
#include <Memory.hpp>
#include <Audio.hpp>

enum Scene
{
	SCENE_FIRST   = 0,
	SCENE_PHONGS  = 1,
	SCENE_SPACE_GAME  = 2,
	SCENE_COUNT
};

class App;
class ExampleScenes
{
public:
	const static inline float pov = 65.0f;
	const static inline float nearPlane = 0.0001f;
	const static inline float farPlane = 1000.0f;

	void Init();
	void Update(float dt);
	void Resize();
	void PostInit() {};
	
	void SetupCamera(Camera t_Camera);
	void RenderSkyBox();
	void RenderDebugGeometry(uint32 t_Id, glm::mat4 t_Translation = glm::mat4(1), glm::mat4 t_Scale = glm::mat4(1), glm::mat4 t_Rotation = glm::mat4(1));
	void RenderDebugGeometryTransform(uint32 t_Id, glm::mat4 t_Transform = glm::mat4(1));

	void ProcessFirstScene(float dt);
	void ProcessPhongScene(float dt);
	void ProcessSpaceScene(float dt);
	
	Graphics* Graphics;
	App* Application;

	TexturedMaterial texMat;
	TexturedMaterialData texMatData;
	PhongMaterial phongMat;
	PhongMaterialData phongMatData;
	BufferDescriptor DebugGeometry;
	Camera camera;
	RasterizationState CurrentRastState;
	LightSetup Light;
	GPUGeometry GPUGeometryDesc;
	Renderer2D Renderer2D;
	SpriteSheetHolder SpriteSheets;
	Scene CurrentScene;
	TextureCatalog Textures;

	AudioPlayer AudioEngine;


	RTObject uiRenderTarget;

};
