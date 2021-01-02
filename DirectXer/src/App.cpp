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

int App::Go()
{


	std::vector<float> cubeVert;
	std::vector<size_t> cubeInd;
	CubeGeometry({}, cubeVert, cubeInd);

	auto vb = m_Graphics.createVertexBuffer(sizeof(SimpleVertex), cubeVert.data(), cubeVert.size() * sizeof(SimpleVertex));
	auto ib = m_Graphics.createIndexBuffer(cubeInd.data(), sizeof(size_t)*cubeInd.size());

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

		auto x = 2.5f * std::sinf(t);
		auto y = 2.5f * std::cosf(t);

		auto projMatrix = dx::XMMatrixPerspectiveFovRH(65.0f, 3.0f/4.0f, 0.0001, 1000.0);
		auto viewMatrix = dx::XMMatrixLookAtRH(
			dx::XMVECTOR{x, 2.5f, y},
				dx::XMVECTOR{0.0, 0.0, 0.0},
				dx::XMVECTOR{0.0, 1.0, 0.0}
			);

		auto modelMatrix = dx::XMMatrixScaling(1.0, 1.0, 1.0)*dx::XMMatrixTranslation(0.0, std::sinf(t), 0.0);

		m_Graphics.updateCB(PSConstantBuffer{ 0.0, 0.0, 1.0, 1.0 });
		m_Graphics.updateCB(VSConstantBuffer{dx::XMMatrixTranspose(modelMatrix*viewMatrix*projMatrix)});

		m_Graphics.drawIndex(Graphics::TT_TRIANGLES, cubeInd.size());


		m_Graphics.EndFrame();

		t += 0.01;


	}

	return m_ReturnValue;
}

void App::DrawFrame()
{


}
