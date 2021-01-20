#include "App.hpp"
#include "Glm.hpp"
#include "Input.hpp"
#include "Logging.hpp"
#include "Math.hpp"
#include "Debug.hpp"

#include <iostream>


struct BufferDescriptor
{
	GeometryBuffer Description;
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

	std::vector<glm::vec3> Colors;

	GeometryBuffer buf;

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

	BufferDescriptor CreateBuffer(Graphics graphics)
	{
		uint16 cube{0};
		uint16 plane{0};
		uint16 cylinder{0};
		uint16 sphere{0};
		uint16 lines{0};

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

static uint32 CUBE;
static uint32 PLANE;
static uint32 LINES;
static uint32 CYLINDER;
static uint32 SPHERE;
static uint32 AXIS;

void App::Init(HWND t_Window)
{

	Graphics.initSwapChain(t_Window, Width, Height);
	Graphics.initBackBuffer();
	Graphics.initZBuffer(Width, Height);
	Graphics.initResources();

	// @Todo: This should some sort of arena storage to do its thing
	BufferBuilder builder;

	CUBE = builder.InitCube(CubeGeometry{}, glm::vec3{1.0f, 0.0f, 0.0f});
	PLANE = builder.InitPlane(PlaneGeometry{}, glm::vec3{0.0f, 1.0f, 0.0f});
	SPHERE = builder.InitSphere(SphereGeometry{}, glm::vec3{0.0f, 1.0f, 0.0f});
	CYLINDER = builder.InitCylinder(CylinderGeometry{0.25, 0.25, 1.5}, glm::vec3{1.0f, 1.0f, 0.0f});
	LINES = builder.InitLines(LinesGeometry{}, glm::vec3{0.0f, 1.0f, 0.0f});


	auto desc = builder.CreateBuffer(Graphics);

	geometryBuffer = desc.Description;

	Graphics.setIndexBuffer(desc.Ibo);
	Graphics.setVertexBuffer(desc.Vbo);


	Graphics.setShaders(Graphics::SHADER_SIMPLE);
	Graphics.setViewport(0, 0, 800, 600);
	Graphics.setRasterizationState();

	camera.Pos = {0.0f, 0.0f, -0.5f};
}

void App::Resize()
{

	Graphics.resizeBackBuffer(Width, Height);
	Graphics.destroyZBuffer();
	Graphics.initZBuffer(Width, Height);
	Graphics.setViewport(0, 0, Width, Height);

}

void App::Spin()
{
	ControlCameraFPS(camera);

	Graphics.ClearBuffer(0.0f, 0.0f, 0.0f);
	Graphics.ClearZBuffer();

	Graphics.m_VertexShaderCB.projection = glm::transpose(glm::perspective(65.0f, 3.0f/4.0f, 0.0001f, 1000.0f));
	Graphics.m_VertexShaderCB.view = glm::transpose(camera.view());

	Graphics.m_PixelShaderCB.color = { 0.0, 1.0, 1.0, 1.0 };

	Graphics.m_VertexShaderCB.model = init_translate(0.5f, 0.0f, 0.0f);
	Graphics.updateCBs();
	geometryBuffer.DrawGeometry(Graphics, CUBE);


	Graphics.m_VertexShaderCB.model = init_translate(-0.5f, 0.0f, 0.0f);
	Graphics.updateCBs();
	geometryBuffer.DrawGeometry(Graphics, PLANE);

	Graphics.m_VertexShaderCB.model = init_scale(0.5f, 0.5f, 0.5f)*init_translate(-1.5f, 0.0f, 0.0f);
	Graphics.updateCBs();
	geometryBuffer.DrawGeometry(Graphics, CYLINDER);

	
	Graphics.m_VertexShaderCB.model = init_scale(0.5f, 0.5f, 0.5f)*init_translate(0.0f, 1.5f, 0.0f);
	Graphics.updateCBs();
	geometryBuffer.DrawGeometry(Graphics, SPHERE);


	Graphics.EndFrame();

}

void App::Destroy()
{
	Graphics.Destroy();
}
