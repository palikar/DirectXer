#pragma once

#include "GeometryUtils.hpp"


static uint32 PutGeometry(BufferDescriptor& buffer, GeometryInfo t_Info)
{
	GPUGeometryInfo newInfo{0};
	newInfo.IndexCount = t_Info.indexCount;
	newInfo.VertexCount = t_Info.vertexCount;
	if (!buffer.Geometries.empty())
	{
		auto lastGeometry = buffer.Geometries.back();
		newInfo.BaseIndex = lastGeometry.VertexCount + lastGeometry.BaseIndex;
		newInfo.IndexOffset = lastGeometry.IndexCount + lastGeometry.IndexOffset;
	}
	
	newInfo.Topology = TopolgyType(t_Info.type >> 8);
	buffer.Geometries.push_back(newInfo);

	return (uint32)buffer.Geometries.size() - 1;
}

void DebugGeometryBuilder::Init(uint8 t_GeometriesCount)
{
	TotalIndices = 0;
	TotalVertices = 0;
	GeometryBuffer.Geometries.reserve(t_GeometriesCount);
	BlobArena = Memory::GetTempArena(Megabytes(4));
}

uint32 DebugGeometryBuilder::InitCube(CubeGeometry t_Cube, glm::vec3 t_Color)
{
	auto cubeInfo = CubeGeometryInfo(t_Cube);

	TotalIndices += cubeInfo.indexCount;
	TotalVertices += cubeInfo.vertexCount;

	BlobArena.Put(GT_CUBE);
	BlobArena.Put(t_Cube);
	BlobArena.Put(cubeInfo);
	BlobArena.Put(t_Color);

	return PutGeometry(GeometryBuffer, cubeInfo);
}

uint32 DebugGeometryBuilder::InitSphere(SphereGeometry t_Sphere, glm::vec3 t_Color)
{
	auto sphereInfo = SphereGeometryInfo(t_Sphere);

	TotalIndices += sphereInfo.indexCount;
	TotalVertices += sphereInfo.vertexCount;

	BlobArena.Put(GT_SPHERE);
	BlobArena.Put(t_Sphere);
	BlobArena.Put(sphereInfo);
	BlobArena.Put(t_Color);

	return PutGeometry(GeometryBuffer, sphereInfo);
}

uint32 DebugGeometryBuilder::InitCylinder(CylinderGeometry t_Cylinder, glm::vec3 t_Color)
{
	auto cylinderInfo = CylinderGeometryInfo(t_Cylinder);

	TotalIndices += cylinderInfo.indexCount;
	TotalVertices += cylinderInfo.vertexCount;

	BlobArena.Put(GT_CYLINDER);
	BlobArena.Put(t_Cylinder);
	BlobArena.Put(cylinderInfo);
	BlobArena.Put(t_Color);

	return PutGeometry(GeometryBuffer, cylinderInfo);
}

uint32 DebugGeometryBuilder::InitPlane(PlaneGeometry t_Plane, glm::vec3 t_Color)
{
	auto planeInfo = PlaneGeometryInfo(t_Plane);

	TotalIndices += planeInfo.indexCount;
	TotalVertices += planeInfo.vertexCount;

	BlobArena.Put(GT_PLANE);
	BlobArena.Put(t_Plane);
	BlobArena.Put(planeInfo);
	BlobArena.Put(t_Color);

	return PutGeometry(GeometryBuffer, planeInfo);
}

uint32 DebugGeometryBuilder::InitLines(LinesGeometry t_Lines, glm::vec3 t_Color)
{
	auto linesInfo = LinesGeometryInfo(t_Lines);

	TotalIndices += linesInfo.indexCount;
	TotalVertices += linesInfo.vertexCount;

	BlobArena.Put(GT_LINES);
	BlobArena.Put(t_Lines);
	BlobArena.Put(linesInfo);
	BlobArena.Put(t_Color);

	return PutGeometry(GeometryBuffer, linesInfo);
}

uint32 DebugGeometryBuilder::InitAxisHelper()
{
	auto axisInfo = AxisHelperInfo();

	TotalIndices += axisInfo.indexCount;
	TotalVertices += axisInfo.vertexCount;

	BlobArena.Put(GT_AXISHELPER);
	BlobArena.Put(axisInfo);

	return PutGeometry(GeometryBuffer, axisInfo);
}

uint32 DebugGeometryBuilder::InitCameraHelper(CameraHelper t_CameraHelper)
{
	auto cameraInfo = CameraHelperInfo(t_CameraHelper);

	TotalIndices += cameraInfo.indexCount;
	TotalVertices += cameraInfo.vertexCount;

	BlobArena.Put(GT_CAMHELPER);
	BlobArena.Put(cameraInfo);
	BlobArena.Put(t_CameraHelper);

	return PutGeometry(GeometryBuffer, cameraInfo);
}

uint32 DebugGeometryBuilder::InitPointLightHelper()
{
	auto lightInfo = PointLightHelperInfo();

	TotalIndices += lightInfo.indexCount;
	TotalVertices += lightInfo.vertexCount;

	BlobArena.Put(GT_POINGHTLIGHTHELPER);
	BlobArena.Put(lightInfo);

	return PutGeometry(GeometryBuffer, lightInfo);
}

uint32 DebugGeometryBuilder::InitSpotLightHelper()
{
	auto lightInfo = SpotLightHelperInfo({});

	TotalIndices += lightInfo.indexCount;
	TotalVertices += lightInfo.vertexCount;

	BlobArena.Put(GT_SPOTLIGHTHELPER);
	BlobArena.Put(lightInfo);

	return PutGeometry(GeometryBuffer, lightInfo);
}

static void SetColor(TempVector<ColorVertex>& t_Vertices, const GeometryInfo& t_Geometry, uint32 offset, glm::vec3 t_Color)
{
	for (size_t i = offset; i < offset + t_Geometry.vertexCount ; i++)
	{
		t_Vertices[i].color = t_Color;
	}		
}
	
IndexedGPUBuffer DebugGeometryBuilder::CreateBuffer(Graphics* graphics)
{
	Memory::EstablishTempScope(Megabytes(4));

	// @Note: This will be the sentinel element at the end of the blob
	BlobArena.Put(GT_UNKNOWN);
		
	char* current = BlobArena.Memory;

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
		
	auto vb = vertexBufferFactory<ColorVertex>(NextVertexBufferId(), *graphics, Vertices);
	auto ib = indexBufferFactory(NextIndexBufferId(), *graphics, Indices);

	graphics->SetVertexBufferName(vb, "DebugGeometryVB");
	graphics->SetIndexBufferName(ib, "DebugGeometryIB");

	Memory::ResetTempScope();

	return {vb, ib};

}
