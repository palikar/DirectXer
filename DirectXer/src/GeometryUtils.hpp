#pragma once

#include "Graphics.hpp"
#include "Geometry.hpp"
#include "GeometryDebug.hpp"
#include "Memory.hpp"

struct BufferDescriptor
{
	BulkVector<GeometryInfo> Infos;

	void DrawGeometry(Graphics graphics, uint32 t_Index)
	{
		uint32 indexOffset = 0;
		uint32 baseIndex = 0;

		for (size_t j = 0; j < t_Index; ++j)
		{
			indexOffset += Infos[j].indexCount;
			baseIndex += Infos[j].vertexCount;
		}

		const uint32 indexCount = Infos[t_Index].indexCount;
		const auto topology = TopolgyType((Infos[t_Index].type & ~255) >> 8);
		graphics.drawIndex(topology, indexCount, indexOffset, baseIndex);

	}

	uint32 PutGeometry(GeometryInfo t_Info)
	{
		Infos.push_back(t_Info);
		return (uint32)Infos.size() - 1;
	}

};

struct GPUGeometry
{
	BufferDescriptor Description;
	VBObject Vbo;
	IBObject Ibo;	
};

template<typename T>
static T& ReadBlob(char* &current)
{
	auto res = (T*)current;
	current += sizeof(T);
	return *res;
}
 
static void SetColor(TempVector<ColorVertex>& t_Vertices, const GeometryInfo& t_Geometry, uint32 offset, glm::vec3 t_Color)
{
	for (size_t i = offset; i < offset + t_Geometry.vertexCount ; i++)
	{
		t_Vertices[i].color = t_Color;
	}
		
}

struct BufferBuilder
{
	BufferDescriptor GeometryBuffer;
	MemoryArena blobArena;
	
	uint32 TotalIndices;
	uint32 TotalVertices;

	void Init(uint8 t_GeometriesCount)
	{
		TotalIndices = 0;
		TotalVertices = 0;
		GeometryBuffer.Infos.reserve(t_GeometriesCount);
		blobArena = Memory::GetTempArena(Megabytes(4));
	}

	uint32 InitCube(CubeGeometry t_Cube, glm::vec3 t_Color)
	{
		auto cubeInfo = CubeGeometryInfo(t_Cube);

		TotalIndices += cubeInfo.indexCount;
		TotalVertices += cubeInfo.vertexCount;

		blobArena.Put(GT_CUBE);
		blobArena.Put(t_Cube);
		blobArena.Put(cubeInfo);
		blobArena.Put(t_Color);

		return GeometryBuffer.PutGeometry(cubeInfo);
	}

	uint32 InitSphere(SphereGeometry t_Sphere, glm::vec3 t_Color)
	{
		auto sphereInfo = SphereGeometryInfo(t_Sphere);

		TotalIndices += sphereInfo.indexCount;
		TotalVertices += sphereInfo.vertexCount;

		blobArena.Put(GT_SPHERE);
		blobArena.Put(t_Sphere);
		blobArena.Put(sphereInfo);
		blobArena.Put(t_Color);

		return GeometryBuffer.PutGeometry(sphereInfo);
	}

	uint32 InitCylinder(CylinderGeometry t_Cylinder, glm::vec3 t_Color)
	{
		auto cylinderInfo = CylinderGeometryInfo(t_Cylinder);

		TotalIndices += cylinderInfo.indexCount;
		TotalVertices += cylinderInfo.vertexCount;

		blobArena.Put(GT_CYLINDER);
		blobArena.Put(t_Cylinder);
		blobArena.Put(cylinderInfo);
		blobArena.Put(t_Color);

		return GeometryBuffer.PutGeometry(cylinderInfo);
	}

	uint32 InitPlane(PlaneGeometry t_Plane, glm::vec3 t_Color)
	{
		auto planeInfo = PlaneGeometryInfo(t_Plane);

		TotalIndices += planeInfo.indexCount;
		TotalVertices += planeInfo.vertexCount;

		blobArena.Put(GT_PLANE);
		blobArena.Put(t_Plane);
		blobArena.Put(planeInfo);
		blobArena.Put(t_Color);

		return GeometryBuffer.PutGeometry(planeInfo);
	}

	uint32 InitLines(LinesGeometry t_Lines, glm::vec3 t_Color)
	{
		auto linesInfo = LinesGeometryInfo(t_Lines);

		TotalIndices += linesInfo.indexCount;
		TotalVertices += linesInfo.vertexCount;

		blobArena.Put(GT_LINES);
		blobArena.Put(t_Lines);
		blobArena.Put(linesInfo);
		blobArena.Put(t_Color);

		return GeometryBuffer.PutGeometry(linesInfo);
	}

	uint32 InitAxisHelper()
	{
		auto axisInfo = AxisHelperInfo();

		TotalIndices += axisInfo.indexCount;
		TotalVertices += axisInfo.vertexCount;

		blobArena.Put(GT_AXISHELPER);
		blobArena.Put(axisInfo);

		return GeometryBuffer.PutGeometry(axisInfo);
	}

	uint32 InitCameraHelper(CameraHelper t_CameraHelper)
	{
		auto cameraInfo = CameraHelperInfo(t_CameraHelper);

		TotalIndices += cameraInfo.indexCount;
		TotalVertices += cameraInfo.vertexCount;

		blobArena.Put(GT_CAMHELPER);
		blobArena.Put(cameraInfo);
		blobArena.Put(t_CameraHelper);

		return GeometryBuffer.PutGeometry(cameraInfo);
	}

	uint32 InitPointLightHelper()
	{
		auto lightInfo = PointLightHelperInfo();

		TotalIndices += lightInfo.indexCount;
		TotalVertices += lightInfo.vertexCount;

		blobArena.Put(GT_POINGHTLIGHTHELPER);
		blobArena.Put(lightInfo);

		return GeometryBuffer.PutGeometry(lightInfo);
	}

	uint32 InitSpotLightHelper()
	{
		auto lightInfo = SpotLightHelperInfo({});

		TotalIndices += lightInfo.indexCount;
		TotalVertices += lightInfo.vertexCount;

		blobArena.Put(GT_SPOTLIGHTHELPER);
		blobArena.Put(lightInfo);

		return GeometryBuffer.PutGeometry(lightInfo);
	}
	
	GPUGeometry CreateBuffer(Graphics graphics)
	{
		Memory::EstablishTempScope(Megabytes(4));

		// @Note: This will be the sentinel element at the end of the blob
		blobArena.Put(GT_UNKNOWN);
		
		char* current = blobArena.Memory;

		// @Note: Use temp types here;
		TempVector<ColorVertex> Vertices;
		TempVector<uint32> Indices;

		Vertices.resize(TotalVertices);
		Indices.reserve(TotalIndices);

		uint32 offset{0};
		
		auto geometryType = *((GeometryType*)current);
		current += sizeof(GeometryType);
		
		while (geometryType != GT_UNKNOWN)
		{
			
			switch(geometryType)
			{
			  case GT_CUBE:
			  {
				  CubeGeometryData(ReadBlob<CubeGeometry>(current), &Vertices[offset], Indices);
				  const auto geometryInfo = ReadBlob<GeometryInfo>(current);
				  SetColor(Vertices, geometryInfo, offset, ReadBlob<glm::vec3>(current));
				  offset += geometryInfo.vertexCount;
				  break;
			  }

			  case GT_PLANE:
			  {
				  PlaneGeometryData(ReadBlob<PlaneGeometry>(current), &Vertices[offset], Indices);
				  const auto geometryInfo = ReadBlob<GeometryInfo>(current);
				  SetColor(Vertices, geometryInfo, offset, ReadBlob<glm::vec3>(current));
				  offset += geometryInfo.vertexCount;
				  break;
			  }

			  case GT_CYLINDER:
			  {
				  CylinderGeometryData(ReadBlob<CylinderGeometry>(current), &Vertices[offset], Indices);
				  const auto geometryInfo = ReadBlob<GeometryInfo>(current);
				  SetColor(Vertices, geometryInfo, offset, ReadBlob<glm::vec3>(current));
				  offset += geometryInfo.vertexCount;
				  break;
			  }

			  case GT_SPHERE:
			  {
				  SphereGeometryData(ReadBlob<SphereGeometry>(current), &Vertices[offset], Indices);
				  const auto geometryInfo = ReadBlob<GeometryInfo>(current);
				  SetColor(Vertices, geometryInfo, offset, ReadBlob<glm::vec3>(current));
				  offset += geometryInfo.vertexCount;
				  break;
			  }

			  case GT_LINES:
			  {
				  LinesGeometryData(ReadBlob<LinesGeometry>(current), &Vertices[offset], Indices);
				  const auto geometryInfo = ReadBlob<GeometryInfo>(current);
				  SetColor(Vertices, geometryInfo, offset, ReadBlob<glm::vec3>(current));
				  offset += geometryInfo.vertexCount;
				  break;
			  }

			  case GT_AXISHELPER:
			  {
				  AxisHelperData(&Vertices[offset], Indices);
				  const auto geometryInfo = ReadBlob<GeometryInfo>(current);
				  offset += geometryInfo.vertexCount;
				  break;
			  }

			  case GT_CAMHELPER:
			  {
				  CameraHelperData({}, &Vertices[offset], Indices);
				  const auto geometryInfo = ReadBlob<GeometryInfo>(current);
				  offset += geometryInfo.vertexCount;
				  break;
			  }

			  case GT_POINGHTLIGHTHELPER:
			  {
				  PointLightHelperData(&Vertices[offset], Indices);
				  const auto geometryInfo = ReadBlob<GeometryInfo>(current);
				  offset += geometryInfo.vertexCount;
				  break;
			  }

			  case GT_SPOTLIGHTHELPER:
			  {
				  SpotLightHelperData({}, &Vertices[offset], Indices);
				  const auto geometryInfo = ReadBlob<GeometryInfo>(current);
				  offset += geometryInfo.vertexCount;
				  break;
			  }

			}

			geometryType = ReadBlob<GeometryType>(current);
		}
		
		auto vb = vertexBufferFactory<ColorVertex>(graphics, Vertices);
		auto ib = indexBufferFactory(graphics, Indices);

		Memory::ResetTempScope();

		return {GeometryBuffer, vb, ib};

	}

};
