#include "App.hpp"
#include "Glm.hpp"
#include "Input.hpp"
#include "Logging.hpp"
#include "Math.hpp"

#include <iostream>


void App::Init(HWND t_Window)
{

	Graphics.initSwapChain(t_Window, Width, Height);
	Graphics.initBackBuffer();
	Graphics.initZBuffer(Width, Height);
	
	Graphics.initResources();


	// cube = CubeGeometryInfo({});
	// plane = CylinderGeometryInfo({1, 1, 5.0, 5.0});
	//sphere = SphereGeometryInfo({2});
	cylinder = CylinderGeometryInfo({});

	// CubeGeometryData({}, plainGeometrVert, plainGeometryInd);
	// SphereGeometryData({2}, plainGeometrVert, plainGeometryInd);
	CylinderGeometryData({}, plainGeometrVert, plainGeometryInd);


	glm::mat4 trans{1};
	trans = glm::rotate(trans, glm::radians(45.0f), glm::vec3(1, 0, 0));
	trans = glm::scale(trans, glm::vec3{ 0.3f, 0.3f, 0.3f });
	TransformVertices(trans, plainGeometrVert.data(), cube.vertexCount/3);

	auto vb = vertexBufferFactory<SimpleVertex>(Graphics, plainGeometrVert);
	auto ib = indexBufferFactory(Graphics, plainGeometryInd);

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
	Graphics.m_VertexShaderCB.model = glm::mat4(1);

	Graphics.updateCBs();
	Graphics.drawIndex(Graphics::TT_TRIANGLES, cylinder.indexCount);

	// Graphics.m_PixelShaderCB.color = { 1.0, 0.0, 0.0, 1.0};
	// Graphics.m_VertexShaderCB.model = init_translate(0.5, 0.0, 0.0);

	// Graphics.updateCBs();
	// Graphics.drawIndex(Graphics::TT_TRIANGLES, plane.indexCount, cube.indexCount, cube.vertexCount/3);

	// Graphics.m_PixelShaderCB.color = { 0.0, 1.0, 0.0, 1.0 };
	// Graphics.m_VertexShaderCB.model = init_translate(3.0, 0.0, 3.0) * init_scale(0.3, 0.3, 0.3);

	// Graphics.updateCBs();
	//Graphics.drawIndex(Graphics::TT_TRIANGLES, sphere.indexCount, cube.indexCount + plane.indexCount, cube.vertexCount/3 + plane.vertexCount/3);

	Graphics.EndFrame();

}

void App::Destroy()
{
	Graphics.Destroy();
}
