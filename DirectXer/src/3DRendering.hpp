#pragma once

#include <Camera.hpp>
#include <Memory.hpp>
#include <Graphics.hpp>
#include <Containers.hpp>
#include <Materials.hpp>
#include <Lighting.hpp>


struct GPUGeometryInfo
{
	uint32 VertexCount;
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
	LightSetup LightingSetup;
	Camera CurrentCamera;
	mat4 CurrentProjection;

	VertexBufferId InstancedBuffer;
	BulkVector<MtlInstanceData> InstancedData;

	DebugGeometryDescription DebugGeometries;

	ShaderConfiguration CurrentConfig;

	ConstantBufferId DebugCBId;

	BulkVector<Vertex3D, Memory_2DRendering> Vertices;
    BulkVector<uint32, Memory_2DRendering> Indices;
    VertexBufferId vbo;
    IndexBufferId ibo;

	Vertex3D* CurrentVertex;
    uint32 CurrentVertexCount;

  public:

	void InitRenderer(Graphics* t_Graphics);
	void InitDebugGeometry(DebugGeometryBuilder& builder);
	void InitLighting();
	void InitInstancedDataBuffer(uint32 maxInstances);

	void SetupProjection(glm::mat4 matrix);

	void DisableLighting();

	void UpdateLighting();
	void UpdateCamera();
	void UpdateInstancedData();
	void UpdateDebugData(float T);

	void BeginScene(ShaderConfiguration config);
	void EndScene();

	void BindLighting();
	void BindMaterial(MaterialId id);
	void BindMaterialInstanced(MaterialId id);


	Lighting&						 AccessLightingData()  { return LightingSetup.LightingData; }
	BulkVector<MtlInstanceData>&	 AccessInstancedData() { return InstancedData;              }

	
	void DrawMesh(MeshId id, float3 pos = {}, float3 scale = {});
	void DrawMesh(MeshId id, mat4 transform);
	
	void DrawSelectedMesh(MeshId id, float3 pos = {}, float3 scale = {});
	void DrawSelectedMesh(MeshId id, mat4 transform);
	
	void DrawInstancedMesh(MeshId id, uint32 instancesCount, uint32 baseInstanced = 0);

	void DrawMeshWithMaterial(MeshId id, float3 pos = {}, float3 scale = {});
	
	void DrawDebugGeometry(uint32 id, float3 pos = {}, float3 scale = {}, glm::mat4 rotation = glm::mat4(1));
	void DrawSelectedDebugGeometry(uint32 id, float3 pos = {}, float3 scale = {}, glm::mat4 rotation = glm::mat4(1));
	void DrawSkyBox(TextureId sky);

	void BeginLines();
	void DrawLine(float3 from, float3 to, float4 color);
	void EndLines();
};
