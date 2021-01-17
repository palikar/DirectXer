#include "App.hpp"
#include "Glm.hpp"
#include "Input.hpp"
#include "Logging.hpp"
#include "Math.hpp"
#include "Debug.hpp"

#include <iostream>


void App::Init(HWND t_Window)
{

	Graphics.initSwapChain(t_Window, Width, Height);
	Graphics.initBackBuffer();
	Graphics.initZBuffer(Width, Height);
	Graphics.initResources();
	

	// Create the infos needed for drawing
	auto axisInfo = AxisHelperInfo();
	auto cubeInfo = CubeGeometryInfo(CubeGeometry{});
	auto planeInfo = PlaneGeometryInfo(PlaneGeometry{});
		
	PutGeometry(geometryBuffer, axisInfo);
	PutGeometry(geometryBuffer, cubeInfo);
	PutGeometry(geometryBuffer, planeInfo);

	// Create actual resources
	std::vector<ColorVertex> debugVertices;
	std::vector<uint32> debugIndices;

	AxisHelperData(debugVertices, debugIndices);

	debugVertices.resize(axisInfo.vertexCount + cubeInfo.vertexCount + planeInfo.vertexCount);

	CubeGeometryData(CubeGeometry{}, &debugVertices[axisInfo.vertexCount].pos.x, debugIndices, sizeof(ColorVertex));
	PlaneGeometryData(PlaneGeometry{}, &debugVertices[axisInfo.vertexCount  + cubeInfo.vertexCount].pos.x, debugIndices, sizeof(ColorVertex));

	for (auto& v : debugVertices)
	{
		v.color = glm::vec3{ 1.0f, 0.0f, 0.0f };
	}	

	auto vb = vertexBufferFactory<ColorVertex>(Graphics, debugVertices);
	auto ib = indexBufferFactory(Graphics, debugIndices);

	Graphics.setVertexBuffer(vb);
	Graphics.setIndexBuffer(ib);

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
	PutDraw(geometryBuffer, 1u, init_translate(1.0f, 0.0f, 0.0f));
	PutDraw(geometryBuffer, 2u, init_translate(-1.0f, 0.0f, 0.0f));


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
