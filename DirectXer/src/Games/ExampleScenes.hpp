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
	SCENE_UI,
	SCENE_OBJECTS,
	SCENE_EDITOR,
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
	
	void UpdateTime(float dt);
	
	void ProcessFirstScene(float dt);
	void ProcessPhongScene(float dt);
	void ProcessBallsScene(float dt);
	void ProcessUIScene(float dt);
	void ProcessObjectsScene(float dt);
	void ProcessEditorScene(float dt);

	struct MeshPrototype
	{
		MeshId Mesh;
		MaterialId Material;
		String Name;
	};
	
	struct MeshEntry
	{
		MeshId Mesh;
		MaterialId Material;
		mat4 Transform;
		String Name;
	};
	
	BulkVector<MeshEntry> Prototypes;
	BulkVector<MeshEntry> Meshes;

	ConstantBufferId DebugCBId;
	
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
