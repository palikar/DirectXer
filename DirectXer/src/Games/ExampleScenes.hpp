#pragma once

#include <Geometry.hpp>
#include <Graphics.hpp>
#include <Camera.hpp>
#include <TextureCatalog.hpp>
#include <GeometryUtils.hpp>
#include <Materials.hpp>
#include <Lighting.hpp>
#include <2DRendering.hpp>
#include <3DRendering.hpp>
#include <Memory.hpp>
#include <Audio.hpp>
#include <Containers.hpp>
#include <Serialization.hpp>

enum Scene
{
	SCENE_FIRST,
	SCENE_PHONGS,
	SCENE_BALLS,
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
	
	void RenderDebugGeometry(uint32 t_Id, glm::mat4 t_Translation = glm::mat4(1), glm::mat4 t_Scale = glm::mat4(1), glm::mat4 t_Rotation = glm::mat4(1)){}

	void UpdateTime(float dt);
	
	void ProcessFirstScene(float dt);
	void ProcessPhongScene(float dt);
	void ProcessBallsScene(float dt);
	// void ProcessObjectsScene(float dt);
	
	Graphics* Graphics;
	App* Application;

	Scene CurrentScene;
	AudioPlayer AudioEngine;
	SerializationContext SaveContext;
	
	RasterizationState CurrentRastState;

	Renderer3D Renderer3D;
	
	Renderer2D Renderer2D;
	SpriteSheetHolder SpriteSheets;
	RTObject uiRenderTarget;

	MaterialId SimplePhong;
	MaterialId RocksTextured;
	MaterialId CheckerTextured;
	MaterialId FloorTextured;

	float T = 0.0f;
	CameraControlState CameraState;
};
