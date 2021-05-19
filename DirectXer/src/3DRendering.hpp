#pragma once

#include <Camera.hpp>
#include <Memory.hpp>
#include <Graphics.hpp>
#include <Containers.hpp>
#include <Materials.hpp>
#include <Lighting.hpp>


struct GPUGeometryInfo
{
	uint32 IndexCount;
	uint32 IndexOffset;
	uint32 BaseIndex;
	TopolgyType Topology;
};

struct GPUGeometry
{
	GPUGeometryInfo Description;
	VertexBufferId Vbo;
	IndexBufferId Ibo;
};

struct BufferDescriptor
{
	TempVector<GPUGeometryInfo> Geometries;
	VertexBufferId Vbo;
	IndexBufferId Ibo;
};

using MeshId = uint32;

struct Mesh
{
	GPUGeometry Geometry;
	MaterialId Material;
};

struct MeshCatalog
{
	Map<MeshId, Mesh, Memory_3DRendering> Meshes;
	MaterialLibrary Materials;
};

struct DebugGeometryBuilder;
class Renderer3D
{
  public:
	struct DebugGeometryDescription
	{
		BulkVector<GPUGeometryInfo> Geometries;
		VertexBufferId Vbo;
		IndexBufferId Ibo;
	};

	Graphics* Gfx;
	MeshCatalog MeshData;
	LightSetup Lighting;
	Camera CurrentCamera;

	DebugGeometryDescription DebugGeometries;

	ShaderConfiguration CurrentConfig;

  public:

	void InitRenderer(Graphics* t_Graphics);
	void InitDebugGeometry(DebugGeometryBuilder& builder);
	void InitLighting();

	void EnableLighting();
	void DisableLighting();
	void UpdateLighting();

	void SetupProjection(glm::mat4 matrix);
	void UpdateCamera();

	void BeginScene(ShaderConfiguration config);
	void EndScene();
	
	void DrawMesh(MeshId id, glm::vec3 pos = {}, glm::vec3 scale = {});
	void DrawDebugGeometry(uint32 id, glm::vec3 pos = {}, glm::vec3 scale = {}, glm::mat4 rotation = {});
	void DrawSkyBox(TextureId sky);
};
