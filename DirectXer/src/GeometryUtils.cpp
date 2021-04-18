#pragma once

#include "GeometryUtils.hpp"

void BufferDescriptor::DrawGeometry(Graphics graphics, uint32 t_Index)
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
	graphics.DrawIndex(topology, indexCount, indexOffset, baseIndex);

}

uint32 BufferDescriptor::PutGeometry(GeometryInfo t_Info)
{
	Infos.push_back(t_Info);
	return (uint32)Infos.size() - 1;
}

static void SetColor(TempVector<ColorVertex>& t_Vertices, const GeometryInfo& t_Geometry, uint32 offset, glm::vec3 t_Color)
{
	for (size_t i = offset; i < offset + t_Geometry.vertexCount ; i++)
	{
		t_Vertices[i].color = t_Color;
	}		
}

void BufferBuilder::Init(uint8 t_GeometriesCount)
{
	TotalIndices = 0;
	TotalVertices = 0;
	GeometryBuffer.Infos.reserve(t_GeometriesCount);
	BlobArena = Memory::GetTempArena(Megabytes(4));
}

uint32 BufferBuilder::InitCube(CubeGeometry t_Cube, glm::vec3 t_Color)
{
	auto cubeInfo = CubeGeometryInfo(t_Cube);

	TotalIndices += cubeInfo.indexCount;
	TotalVertices += cubeInfo.vertexCount;

	BlobArena.Put(GT_CUBE);
	BlobArena.Put(t_Cube);
	BlobArena.Put(cubeInfo);
	BlobArena.Put(t_Color);

	return GeometryBuffer.PutGeometry(cubeInfo);
}

uint32 BufferBuilder::InitSphere(SphereGeometry t_Sphere, glm::vec3 t_Color)
{
	auto sphereInfo = SphereGeometryInfo(t_Sphere);

	TotalIndices += sphereInfo.indexCount;
	TotalVertices += sphereInfo.vertexCount;

	BlobArena.Put(GT_SPHERE);
	BlobArena.Put(t_Sphere);
	BlobArena.Put(sphereInfo);
	BlobArena.Put(t_Color);

	return GeometryBuffer.PutGeometry(sphereInfo);
}

uint32 BufferBuilder::InitCylinder(CylinderGeometry t_Cylinder, glm::vec3 t_Color)
{
	auto cylinderInfo = CylinderGeometryInfo(t_Cylinder);

	TotalIndices += cylinderInfo.indexCount;
	TotalVertices += cylinderInfo.vertexCount;

	BlobArena.Put(GT_CYLINDER);
	BlobArena.Put(t_Cylinder);
	BlobArena.Put(cylinderInfo);
	BlobArena.Put(t_Color);

	return GeometryBuffer.PutGeometry(cylinderInfo);
}

uint32 BufferBuilder::InitPlane(PlaneGeometry t_Plane, glm::vec3 t_Color)
{
	auto planeInfo = PlaneGeometryInfo(t_Plane);

	TotalIndices += planeInfo.indexCount;
	TotalVertices += planeInfo.vertexCount;

	BlobArena.Put(GT_PLANE);
	BlobArena.Put(t_Plane);
	BlobArena.Put(planeInfo);
	BlobArena.Put(t_Color);

	return GeometryBuffer.PutGeometry(planeInfo);
}

uint32 BufferBuilder::InitLines(LinesGeometry t_Lines, glm::vec3 t_Color)
{
	auto linesInfo = LinesGeometryInfo(t_Lines);

	TotalIndices += linesInfo.indexCount;
	TotalVertices += linesInfo.vertexCount;

	BlobArena.Put(GT_LINES);
	BlobArena.Put(t_Lines);
	BlobArena.Put(linesInfo);
	BlobArena.Put(t_Color);

	return GeometryBuffer.PutGeometry(linesInfo);
}

uint32 BufferBuilder::InitAxisHelper()
{
	auto axisInfo = AxisHelperInfo();

	TotalIndices += axisInfo.indexCount;
	TotalVertices += axisInfo.vertexCount;

	BlobArena.Put(GT_AXISHELPER);
	BlobArena.Put(axisInfo);

	return GeometryBuffer.PutGeometry(axisInfo);
}

uint32 BufferBuilder::InitCameraHelper(CameraHelper t_CameraHelper)
{
	auto cameraInfo = CameraHelperInfo(t_CameraHelper);

	TotalIndices += cameraInfo.indexCount;
	TotalVertices += cameraInfo.vertexCount;

	BlobArena.Put(GT_CAMHELPER);
	BlobArena.Put(cameraInfo);
	BlobArena.Put(t_CameraHelper);

	return GeometryBuffer.PutGeometry(cameraInfo);
}

uint32 BufferBuilder::InitPointLightHelper()
{
	auto lightInfo = PointLightHelperInfo();

	TotalIndices += lightInfo.indexCount;
	TotalVertices += lightInfo.vertexCount;

	BlobArena.Put(GT_POINGHTLIGHTHELPER);
	BlobArena.Put(lightInfo);

	return GeometryBuffer.PutGeometry(lightInfo);
}

uint32 BufferBuilder::InitSpotLightHelper()
{
	auto lightInfo = SpotLightHelperInfo({});

	TotalIndices += lightInfo.indexCount;
	TotalVertices += lightInfo.vertexCount;

	BlobArena.Put(GT_SPOTLIGHTHELPER);
	BlobArena.Put(lightInfo);

	return GeometryBuffer.PutGeometry(lightInfo);
}
	
GPUGeometry BufferBuilder::CreateBuffer(Graphics& graphics)
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
		
	auto vb = vertexBufferFactory<ColorVertex>(NextVertexBufferId(), graphics, Vertices);
	auto ib = indexBufferFactory(NextIndexBufferId(), graphics, Indices);

	Memory::ResetTempScope();

	return {GeometryBuffer, vb, ib};

}
