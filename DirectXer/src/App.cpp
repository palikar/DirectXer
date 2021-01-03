#include "App.h"

#include "Geometry.hpp"
#include <iostream>

App::App() :
	m_Window(800, 600, "DirectXer")
{

	m_Graphics.initSwapChain(m_Window.hWnd);
	m_Graphics.initBackBuffer();
	m_Graphics.initZBuffer();

	m_Graphics.initResources();


}

void App::processMessages()
{

	if (const auto ecode = Window::ProcessMessages())
	{
		m_ReturnValue  = *ecode;
		m_Running = false;
	}

}

template<typename VertexType>
VBObject vertexBufferFactory(Graphics& graphics, std::vector<float> t_VertexList)
{
	return graphics.createVertexBuffer(sizeof(VertexType), t_VertexList.data(), sizeof(VertexType)*t_VertexList.size());
}

template<typename IndexType = size_t>
IBObject indexBufferFactory(Graphics& graphics, std::vector<IndexType> t_IndexList)
{
	return graphics.createIndexBuffer(t_IndexList.data(), sizeof(IndexType)*t_IndexList.size());
}


int App::Go()
{

	std::vector<float> plainGeometrVert;
	std::vector<size_t> plainGeometryInd;
	auto cube = CubeGeometryInfo({});
	auto plane = PlaneGeometryInfo({1, 1, 5.0, 5.0});
	auto sphere = SphereGeometryInfo({2});
	
	CubeGeometry({}, plainGeometrVert, plainGeometryInd);
	PlaneGeometry({1, 1, 5.0, 5.0}, plainGeometrVert, plainGeometryInd);
	SphereGeometry({2}, plainGeometrVert, plainGeometryInd);

	auto vb = vertexBufferFactory<SimpleVertex>(m_Graphics, plainGeometrVert);
	auto ib = indexBufferFactory(m_Graphics, plainGeometryInd);

	m_Graphics.setVertexBuffer(vb);
	m_Graphics.setIndexBuffer(ib);

	m_Graphics.setShaders(Graphics::SHADER_SIMPLE);
	m_Graphics.setViewport(0, 0, 800, 600);
	m_Graphics.setRasterizationState();

	float t = 0.0;
	while (m_Running)
	{
		processMessages();

		m_Graphics.ClearBuffer(0.0f, 0.0f, 0.0f);
		m_Graphics.ClearZBuffer();

		auto x = 7.5f * std::sinf(t);
		auto y = 7.5f * std::cosf(t);

		m_Graphics.m_VertexShaderCB.projection = dx::XMMatrixTranspose(
			dx::XMMatrixPerspectiveFovRH(65.0f, 3.0f/4.0f, 0.0001, 1000.0)
		);

		m_Graphics.m_VertexShaderCB.view = dx::XMMatrixTranspose(
			dx::XMMatrixLookAtRH(
				dx::XMVECTOR{x, 7.5f, y},
				dx::XMVECTOR{0.0, 0.0, 0.0},
				dx::XMVECTOR{0.0, 1.0, 0.0}
		));


		m_Graphics.m_PixelShaderCB.color = { 0.0, 1.0, 1.0, 1.0 };
		m_Graphics.m_VertexShaderCB.model = dx::XMMatrixTranspose(
			dx::XMMatrixScaling(1.0, 1.0, 1.0)*dx::XMMatrixTranslation(0.0, 0.0, 0.0)
		);

		m_Graphics.updateCBs();
		m_Graphics.drawIndex(Graphics::TT_TRIANGLES, cube.indexCount);


		m_Graphics.m_PixelShaderCB.color = { 1.0, 0.0, 0.0, 1.0};
		m_Graphics.m_VertexShaderCB.model = dx::XMMatrixTranspose(
			dx::XMMatrixScaling(1.0, 1.0, 1.0)*dx::XMMatrixTranslation(0.0, 0.0, 0.0)
		);
		

		m_Graphics.updateCBs();
		m_Graphics.drawIndex(Graphics::TT_TRIANGLES, plane.indexCount, cube.indexCount, cube.vertexCount/3);


		m_Graphics.m_PixelShaderCB.color = { 0.0, 1.0, 0.0, 1.0 };
		m_Graphics.m_VertexShaderCB.model = dx::XMMatrixTranspose(
			dx::XMMatrixScaling(0.3, 0.3, 0.3)*dx::XMMatrixTranslation(2.0, 0.0, 0.0)
		);
		

		m_Graphics.updateCBs();
		m_Graphics.drawIndex(Graphics::TT_TRIANGLES, sphere.indexCount, cube.indexCount + plane.indexCount, cube.vertexCount/3 + plane.vertexCount/3);

		m_Graphics.EndFrame();

		t += 0.01;


	}

	return m_ReturnValue;
}
