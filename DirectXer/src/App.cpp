#include "App.hpp"
#include "Glm.hpp"
#include "Input.hpp"
#include "Logging.hpp"
#include "Math.hpp"
#include "Debug.hpp"

#include <iostream>


enum GeometryType
{
	GT_CUBE = 1,
	GT_PLANE = 2,
	GT_AXISHELPER = 4,
};


struct BufferBuilder
{

	std::vector<GeometryType> Objects;
	std::vector<GeometryInfo> Geometries;

	std::vector<CubeGeometry> Cubes;
	std::vector<PlaneGeometry> Planes;

	std::vector<glm::vec3> Colors;
	GeometryBuffer buf;
	
	
	uint32 TotalIndices{0};
	uint32 TotalVertices{0};
	
	uint32 InitCube(CubeGeometry t_Cube, glm::vec3 t_Color)
	{
		auto cubeInfo = CubeGeometryInfo(t_Cube);

		TotalIndices += cubeInfo.indexCount;
		TotalVertices += cubeInfo.vertexCount;

		Objects.push_back(GT_CUBE);
		Cubes.push_back(t_Cube);
		Geometries.push_back(cubeInfo);

		Colors.push_back(t_Color);

		return PutGeometry(buf, cubeInfo);
	}

	uint32 InitPlane(PlaneGeometry t_Plane, glm::vec3 t_Color)
	{
		auto planeInfo = PlaneGeometryInfo(t_Plane);

		TotalIndices += planeInfo.indexCount;
		TotalVertices += planeInfo.vertexCount;
		
		Objects.push_back(GT_PLANE);
		Planes.push_back(t_Plane);
		Geometries.push_back(planeInfo);

		Colors.push_back(t_Color);

		return PutGeometry(buf, planeInfo);
	}
	
	

	GeometryBuffer CreateBuffer(Graphics graphics)
	{
		
		

		uint16 cube{0};
		uint16 plane{0};

		uint32 offset{0};
		uint32 color{0};
		
		uint32 geometry{0};

		std::vector<ColorVertex> Vertices;
		std::vector<uint32> Indices;
	
		Vertices.resize(TotalVertices);
		Indices.reserve(TotalIndices);
		
		for (auto obj : Objects)
		{

			switch(obj)
			{
			  case GT_CUBE:
			  {
					CubeGeometryData(Cubes[cube++], &Vertices[offset].pos.x, Indices, sizeof(ColorVertex));
					for (size_t i = offset; i < offset + Geometries[geometry++].vertexCount; i++)
					{
						Vertices[i].color = Colors[color];
					}
				  
					offset += Geometries[geometry++].vertexCount; 
					++color;
					break;
			  }
			  case GT_PLANE:
			  {
				  PlaneGeometryData(Planes[plane++], &Vertices[offset].pos.x, Indices, sizeof(ColorVertex));
				  offset += Geometries[geometry++].vertexCount;
				  break;
			  }

			}
			
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
	

	// Create the infos needed for drawing
	//auto axisInfo = AxisHelperInfo();
	//auto cubeInfo = CubeGeometryInfo();
	//auto planeInfo = PlaneGeometryInfo(PlaneGeometry{});

	BufferBuilder builder;

	builder.InitCube(CubeGeometry{}, glm::vec3{1.0f, 0.0f, 0.0f});
	builder.InitPlane(PlaneGeometry{}, glm::vec3{0.0f, 1.0f, 0.0f});

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
	

	 PutDraw(geometryBuffer, 0u, init_translate(0.0f, 0.0f, 0.0f));
	// PutDraw(geometryBuffer, 1u, init_translate(1.0f, 0.0f, 0.0f));
	// PutDraw(geometryBuffer, 2u, init_translate(-1.0f, 0.0f, 0.0f));


	for (size_t i = 0; i < geometryBuffer.Draws.size(); ++i)
	{
		const auto &draw = geometryBuffer.Draws[i];

		Graphics.m_VertexShaderCB.model = draw.Transform;
		Graphics.updateCBs();

		uint32 indexCount = geometryBuffer.Infos[draw.Index].indexCount;
		uint32 indexOffset = 0;
		uint32 baseIndex = 0;
		
		for (size_t j = 0; j < draw.Index; ++j)
		{
			indexOffset += geometryBuffer.Infos[j].indexCount;
			baseIndex += geometryBuffer.Infos[j].vertexCount;
		}
		Graphics.drawIndex(Graphics::TT_TRIANGLES, indexCount, indexOffset, baseIndex);
	}


	// Graphics.m_PixelShaderCB.color = { 1.0, 0.0, 0.0, 1.0};
	// Graphics.m_VertexShaderCB.model = init_translate(0.5, 0.0, 0.0);

	// Graphics.updateCBs();
	// Graphics.drawIndex(Graphics::TT_TRIANGLES, plane.indexCount, cube.indexCount, cube.vertexCount/3);

	ResetBuffer(geometryBuffer);
	Graphics.EndFrame();

}

void App::Destroy()
{
	Graphics.Destroy();
}
