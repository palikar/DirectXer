#include "App.hpp"
#include "Glm.hpp"
#include "Input.hpp"
#include "Logging.hpp"
#include "Math.hpp"

#include <iostream>

static GeometryInfo AxisHelperInfo()
{
	// @Todo: This is a number that is know all the time!
	CylinderGeometry cylinderShape{0.005f, 0.005f, 0.25f, 10.0f};
	auto cylinderInfo = CylinderGeometryInfo(cylinderShape);
	return {cylinderInfo.vertexCount*3, cylinderInfo.indexCount*3};
}

static int AxisHelperData(std::vector<ColorVertex>& t_Vertices, std::vector<uint32>& t_Indices)
{

	// @Todo: We _know_ the numbers that are in here
	CylinderGeometry cylinderShape{0.005f, 0.005f, 0.25f, 10.0f};
	auto cylinderInfo = CylinderGeometryInfo(cylinderShape);

	// @Todo: We can make everything without this vector
	std::vector<ColorVertex> vertices;
	vertices.resize(cylinderInfo.vertexCount*3);

	CylinderGeometryData(cylinderShape, &vertices[cylinderInfo.vertexCount*0].pos.x, t_Indices, sizeof(ColorVertex), cylinderInfo.vertexCount * 0u);
	CylinderGeometryData(cylinderShape, &vertices[cylinderInfo.vertexCount*1].pos.x, t_Indices, sizeof(ColorVertex), cylinderInfo.vertexCount * 1u);
	CylinderGeometryData(cylinderShape, &vertices[cylinderInfo.vertexCount*2].pos.x, t_Indices, sizeof(ColorVertex), cylinderInfo.vertexCount * 2u);

	glm::mat4 transform(1);
	transform = glm::translate(transform, glm::vec3(0.0f, 0.25f * 0.5f, 0.0f));
	TransformVertices(transform, &vertices[cylinderInfo.vertexCount*0], cylinderInfo.vertexCount);
	
	transform = glm::mat4(1);
	transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(1, 0, 0));
	transform = glm::translate(transform, glm::vec3(0.0f, 0.25f * 0.5f, 0.0f));
	TransformVertices(transform, &vertices[cylinderInfo.vertexCount*1], cylinderInfo.vertexCount);

	transform = glm::mat4(1);
	transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(0, 0, 1));
	transform = glm::translate(transform, glm::vec3(0.0f, 0.25f * 0.5f, 0.0f));
	TransformVertices(transform, &vertices[cylinderInfo.vertexCount*2], cylinderInfo.vertexCount);
	
	for (size_t i = 0; i < cylinderInfo.vertexCount; ++i)
	{
		vertices[i].color = glm::vec3{0.0f, 1.0f, 0.0f};
	}

	for (size_t i = cylinderInfo.vertexCount; i < cylinderInfo.vertexCount*2; ++i)
	{
		vertices[i].color = glm::vec3{1.0f, 0.0f, 0.0f};
	}

	for (size_t i = cylinderInfo.vertexCount*2; i < cylinderInfo.vertexCount*3; ++i)
	{
		vertices[i].color = glm::vec3{0.0f, 0.0f, 1.0f};
	}
	
	t_Vertices.insert(t_Vertices.end(), vertices.begin(), vertices.end());
	
	return 0;
}

void App::Init(HWND t_Window)
{

	Graphics.initSwapChain(t_Window, Width, Height);
	Graphics.initBackBuffer();
	Graphics.initZBuffer(Width, Height);
	Graphics.initResources();
	

	std::vector<ColorVertex> debugVertices;
	std::vector<uint32> debugIndices;

	axisHelper = AxisHelperInfo();
	
	AxisHelperData(debugVertices, debugIndices);
	
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
	Graphics.m_VertexShaderCB.model = glm::mat4(1);

	Graphics.updateCBs();
	Graphics.drawIndex(Graphics::TT_TRIANGLES, axisHelper.indexCount);

	// Graphics.m_PixelShaderCB.color = { 1.0, 0.0, 0.0, 1.0};
	// Graphics.m_VertexShaderCB.model = init_translate(0.5, 0.0, 0.0);

	// Graphics.updateCBs();
	// Graphics.drawIndex(Graphics::TT_TRIANGLES, plane.indexCount, cube.indexCount, cube.vertexCount/3);


	Graphics.EndFrame();

}

void App::Destroy()
{
	Graphics.Destroy();
}
