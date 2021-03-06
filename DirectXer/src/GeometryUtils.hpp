#pragma once

#include <Graphics.hpp>
#include <Geometry.hpp>
#include <GeometryDebug.hpp>
#include <Memory.hpp>
#include <Containers.hpp>
#include <3DRendering.hpp>

struct IndexedGPUBuffer
{
	VertexBufferId vbo;
	IndexBufferId ibo;
};

struct DebugGeometryBuilder
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
	
	IndexedGPUBuffer CreateBuffer(Graphics* graphics);
};


