#include "App.hpp"
#include "Glm.hpp"
#include "Input.hpp"
#include "Logging.hpp"
#include "Math.hpp"
#include "Debug.hpp"

#include <iostream>


struct BufferBuilder
{
	std::vector<GeometryInfo> Geometries;

	// @Todo: all of that stuff should be placed on some random ass memory
	std::vector<CubeGeometry> Cubes;
	std::vector<PlaneGeometry> Planes;
	std::vector<CylinderGeometry> Cylinders;
	std::vector<SphereGeometry> Spheres;
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

	GeometryBuffer CreateBuffer(Graphics graphics)
	{
		uint16 cube{0};
		uint16 plane{0};

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
					CubeGeometryData(Cubes[cube++], &Vertices[offset], Indices, sizeof(ColorVertex));
					break;
			  }

			  case GT_PLANE:
			  {
				  PlaneGeometryData(Planes[plane++], &Vertices[offset], Indices, sizeof(ColorVertex));
				  break;
			  }

			  case GT_CYLINDER:
			  {
				  CylinderGeometryData(Cylinders[cylinder++], &Vertices[offset], Indices, sizeof(ColorVertex));
				  break;
			  }

			  case GT_SPHERE:
			  {
				  SphereGeometryData(Sphere[cylinder++], &Vertices[offset], Indices, sizeof(ColorVertex));
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

		graphics.setVertexBuffer(vb);
		graphics.setIndexBuffer(ib);

		return buf;

	}

};

void App::Init(HWND t_Window)
{

	Graphics.initSwapChain(t_Window, Width, Height);
	Graphics.initBackBuffer();
	Graphics.initZBuffer(Width, Height);
	Graphics.initResources();

    // @Todo: This should some sort of arena storage to do its thing
	BufferBuilder builder;

	builder.InitCube(CubeGeometry{}, glm::vec3{1.0f, 0.0f, 0.0f});
	builder.InitPlane(PlaneGeometry{}, glm::vec3{0.0f, 1.0f, 0.0f});

	// @Todo: This should return "BufferDescriptor" -- VBO, IBO, GeometryDescriptor
	geometryBuffer = builder.CreateBuffer(Graphics);

	
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
	geometryBuffer.DrawGeometry(Graphics, 0);

	
	Graphics.m_VertexShaderCB.model = init_translate(-0.5f, 0.0f, 0.0f);
	Graphics.updateCBs();
	geometryBuffer.DrawGeometry(Graphics, 1);

	Graphics.EndFrame();

}

void App::Destroy()
{
	Graphics.Destroy();
}
