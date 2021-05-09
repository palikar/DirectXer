#pragma once

#include <Graphics.hpp>
#include <Geometry.hpp>
#include <GeometryDebug.hpp>
#include <Memory.hpp>
#include <Containers.hpp>
#include <Materials.hpp>

struct BufferDescriptor
{
	BulkVector<GeometryInfo> Infos;

	void DrawGeometry(Graphics& graphics, uint32 t_Index);
	uint32 PutGeometry(GeometryInfo t_Info);
};

struct GPUGeometry
{
	BufferDescriptor Description;
	VertexBufferId Vbo;
	IndexBufferId Ibo;	
};

struct BufferBuilder
{
	BufferDescriptor GeometryBuffer;
	MemoryArena BlobArena;
	
	uint32 TotalIndices;
	uint32 TotalVertices;

	void Init(uint8 t_GeometriesCount);

	uint32 InitCube(CubeGeometry t_Cube, glm::vec3 t_Color);
	uint32 InitSphere(SphereGeometry t_Sphere, glm::vec3 t_Color);
	uint32 InitCylinder(CylinderGeometry t_Cylinder, glm::vec3 t_Color);
	uint32 InitPlane(PlaneGeometry t_Plane, glm::vec3 t_Color);
	uint32 InitLines(LinesGeometry t_Lines, glm::vec3 t_Color);
	uint32 InitAxisHelper();
	uint32 InitCameraHelper(CameraHelper t_CameraHelper);
	uint32 InitPointLightHelper();
	uint32 InitSpotLightHelper();
	
	GPUGeometry CreateBuffer(Graphics& graphics);

};

struct MeshGeometryInfo
{
	VertexBufferId VertexBuffer;
	IndexBufferId IndexBuffer;
	uint32 IndexCount{0};
};

using MeshId = uint32;

struct Mesh
{
	MeshGeometryInfo Geometry;
	MaterialId Material;
};

struct MeshCatalog
{
	Map<MeshId, Mesh> Meshes;
	Map<MaterialId, MtlMaterial> Materials;
	
	Graphics* Gfx;

	void Init(Graphics* graphics)
	{
		Gfx = graphics;
		Meshes.reserve(32);
	}

	void DrawMesh(MeshId id, glm::vec3 pos = {}, glm::vec3 scale = {}, ConstantBufferId light ={})
	{
		const auto mesh = Meshes.at(id);
		const auto material = Materials.at(mesh.Material);

 		Gfx->SetShaderConfiguration(material.Program);

		Gfx->BindVSConstantBuffers(material.Cbo, 1);
		Gfx->BindVSConstantBuffers(light, 2);
		
		Gfx->BindVertexBuffer(mesh.Geometry.VertexBuffer);
		Gfx->BindIndexBuffer(mesh.Geometry.IndexBuffer);
		
		Gfx->VertexShaderCB.model = init_translate(pos) * init_scale(scale);
		Gfx->VertexShaderCB.invModel = glm::inverse(Gfx->VertexShaderCB.model);
		Gfx->UpdateCBs();

		Gfx->BindVSTexture(0, material.KaMap);
		Gfx->BindVSTexture(1, material.KdMap);

		Gfx->DrawIndex(TT_TRIANGLES, mesh.Geometry.IndexCount, 0, 0);
	}
};
