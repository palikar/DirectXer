#pragma once

#include <Graphics.hpp>
#include <Geometry.hpp>
#include <GeometryDebug.hpp>
#include <Memory.hpp>
#include <Containers.hpp>

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
};

struct MeshCatalog
{
	Map<MeshId, Mesh> Meshes;
	Graphics* Gfx;

	void Init(Graphics* graphics)
	{
		Gfx = graphics;
		Meshes.reserve(32);
	}

	void DrawMesh(MeshId id, glm::vec3 pos = {}, glm::vec3 scale = {})
	{
		const auto mesh = Meshes.at(id);

		Gfx->VertexShaderCB.model = init_translate(pos) * init_scale(scale);
		Gfx->VertexShaderCB.invModel = glm::inverse(Gfx->VertexShaderCB.model);
		Gfx->UpdateCBs();
		
		Gfx->BindVertexBuffer(mesh.Geometry.VertexBuffer);
		Gfx->BindIndexBuffer(mesh.Geometry.IndexBuffer);
		Gfx->SetShaderConfiguration(SC_DEBUG_COLOR);
		Gfx->DrawIndex(TT_TRIANGLES, mesh.Geometry.IndexCount, 0, 0);
	}
};
