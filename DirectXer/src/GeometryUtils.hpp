#pragma once

#include "Graphics.hpp"
#include "Geometry.hpp"
#include "GeometryDebug.hpp"

struct BufferDescriptor
{
	// @Todo: This shold be something that does not allocate!
	std::vector<GeometryInfo> Infos;

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
		const auto topology = Graphics::TopolgyType((Infos[t_Index].type & ~255) >> 8);
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

struct BufferBuilder
{
	std::vector<GeometryInfo> Geometries;

	// @Todo: all of that stuff should be placed on some random ass memory
	std::vector<CubeGeometry> Cubes;
	std::vector<PlaneGeometry> Planes;
	std::vector<CylinderGeometry> Cylinders;
	std::vector<SphereGeometry> Spheres;
	std::vector<LinesGeometry> Lines;

	std::vector<CameraHelper> Cameras;


	std::vector<glm::vec3> Colors;

	BufferDescriptor buf;

	uint32 TotalIndices{0};
	uint32 TotalVertices{0};

	uint32 InitCube(CubeGeometry t_Cube, glm::vec3 t_Color)
	{
		auto cubeInfo = CubeGeometryInfo(t_Cube);

		TotalIndices += cubeInfo.indexCount;
		TotalVertices += cubeInfo.vertexCount;

		Cubes.push_back(t_Cube);
		Geometries.push_back(cubeInfo);

		Colors.push_back(t_Color);

		return buf.PutGeometry(cubeInfo);
	}

	uint32 InitSphere(SphereGeometry t_Sphere, glm::vec3 t_Color)
	{
		auto sphereInfo = SphereGeometryInfo(t_Sphere);

		TotalIndices += sphereInfo.indexCount;
		TotalVertices += sphereInfo.vertexCount;

		Spheres.push_back(t_Sphere);
		Geometries.push_back(sphereInfo);

		Colors.push_back(t_Color);

		return buf.PutGeometry(sphereInfo);
	}

	uint32 InitCylinder(CylinderGeometry t_Cylinder, glm::vec3 t_Color)
	{
		auto cylinderInfo = CylinderGeometryInfo(t_Cylinder);

		TotalIndices += cylinderInfo.indexCount;
		TotalVertices += cylinderInfo.vertexCount;

		Cylinders.push_back(t_Cylinder);
		Geometries.push_back(cylinderInfo);

		Colors.push_back(t_Color);

		return buf.PutGeometry(cylinderInfo);
	}

	uint32 InitPlane(PlaneGeometry t_Plane, glm::vec3 t_Color)
	{
		auto planeInfo = PlaneGeometryInfo(t_Plane);

		TotalIndices += planeInfo.indexCount;
		TotalVertices += planeInfo.vertexCount;

		Planes.push_back(t_Plane);
		Geometries.push_back(planeInfo);

		Colors.push_back(t_Color);

		return buf.PutGeometry(planeInfo);
	}

	uint32 InitLines(LinesGeometry t_Lines, glm::vec3 t_Color)
	{
		auto linesInfo = LinesGeometryInfo(t_Lines);

		TotalIndices += linesInfo.indexCount;
		TotalVertices += linesInfo.vertexCount;

		Lines.push_back(t_Lines);
		Geometries.push_back(linesInfo);

		Colors.push_back(t_Color);

		return buf.PutGeometry(linesInfo);
	}

	uint32 InitAxisHelper()
	{
		auto axisInfo = AxisHelperInfo();

		TotalIndices += axisInfo.indexCount;
		TotalVertices += axisInfo.vertexCount;

		Geometries.push_back(axisInfo);

		return buf.PutGeometry(axisInfo);
	}

	uint32 InitCameraHelper(CameraHelper t_CameraHelper)
	{
		auto cameraInfo = CameraHelperInfo(t_CameraHelper);

		TotalIndices += cameraInfo.indexCount;
		TotalVertices += cameraInfo.vertexCount;

		Geometries.push_back(cameraInfo);

		Cameras.push_back(t_CameraHelper);

		return buf.PutGeometry(cameraInfo);
	}

	GPUGeometry CreateBuffer(Graphics graphics)
	{
		uint16 cube{0};
		uint16 plane{0};
		uint16 cylinder{0};
		uint16 sphere{0};
		uint16 lines{0};
		uint16 camera{0};

		uint32 offset{0};
		uint32 color{0};

		std::vector<ColorVertex> Vertices;
		std::vector<uint32> Indices;

		Vertices.resize(TotalVertices);
		Indices.reserve(TotalIndices);

		for (auto& geometry : Geometries)
		{

			switch(geometry.type)
			{
			  case GT_CUBE:
			  {
					CubeGeometryData(Cubes[cube++], &Vertices[offset], Indices);
					break;
			  }

			  case GT_PLANE:
			  {
				  PlaneGeometryData(Planes[plane++], &Vertices[offset], Indices);
				  break;
			  }

			  case GT_CYLINDER:
			  {
				  CylinderGeometryData(Cylinders[cylinder++], &Vertices[offset], Indices);
				  break;
			  }

			  case GT_SPHERE:
			  {
				  SphereGeometryData(Spheres[sphere++], &Vertices[offset], Indices);
				  break;
			  }

			  case GT_LINES:
			  {
				  LinesGeometryData(Lines[lines++], &Vertices[offset], Indices);
				  break;
			  }

			  case GT_AXISHELPER:
			  {
				  AxisHelperData(&Vertices[offset], Indices);
				  offset += geometry.vertexCount;
				  continue;
			  }

			  case GT_CAMHELPER:
			  {
				  CameraHelperData(Cameras[camera++], &Vertices[offset], Indices);
				  offset += geometry.vertexCount;
				  continue;
			  }

			  case GT_UNKNOWN:
			  {
				  // @Todo: Warning here!
				  break;
			  }

			}

			for (size_t i = offset; i < offset + geometry.vertexCount ; i++)
			{
				Vertices[i].color = Colors[color];
			}

			offset += geometry.vertexCount;
			++color;

		}

		auto vb = vertexBufferFactory<ColorVertex>(graphics, Vertices);
		auto ib = indexBufferFactory(graphics, Indices);

		return {buf, vb, ib};

	}

};
