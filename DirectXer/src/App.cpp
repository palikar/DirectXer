#include "App.h"

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

	SimpleVertex vertices[] = {
		{-0.5, -0.5, 0.0},
		{-0.5, 0.5, 0.0},
		{0.5, -0.5, 0.0},
		{0.5, 0.5, 0.0},
	};

	size_t indices[] = {
		0,1,2,
		1,2,3,
	};


	auto vb = m_Graphics.createVertexBuffer(sizeof(SimpleVertex), vertices, sizeof(vertices));
	auto ib = m_Graphics.createIndexBuffer(indices, sizeof(indices));

	m_Graphics.setVertexBuffer(vb);
	m_Graphics.setIndexBuffer(ib);

	m_Graphics.setShaders(Graphics::SHADER_SIMPLE);
	m_Graphics.setViewport(0, 0, 800, 600);
	m_Graphics.setRasterizationState();

	float t = 0.0;
	while (m_Running)
	{
		processMessages();

		m_Graphics.ClearBuffer(0.0f, 1.0f, 0.0f);
		m_Graphics.ClearZBuffer();


		m_Graphics.updateCB(PSConstantBuffer{1.0, std::sin(t), 0.0, 1.0});
		m_Graphics.updateCB(VSConstantBuffer{
			dx::XMMatrixTranspose(
				dx::XMMatrixRotationZ(t)*
				dx::XMMatrixScaling((3.0/4.0)*1.0, 1.0, 1.0)*
				dx::XMMatrixTranslation(0.0, 0.0, 0.5)
			)
		});
		m_Graphics.drawIndex(Graphics::TT_TRIANGLES, 6);



		m_Graphics.updateCB(PSConstantBuffer{ 0.0, 0.0, 1.0, 1.0 });
		m_Graphics.updateCB(VSConstantBuffer{
			dx::XMMatrixTranspose(
				dx::XMMatrixScaling((3.0 / 4.0) * 0.5, 0.5, 0.5) *
				dx::XMMatrixTranslation(0.0, 0.0, -0.5)
			)
			});

		m_Graphics.drawIndex(Graphics::TT_TRIANGLES, 6);


		m_Graphics.EndFrame();

		t += 0.01;


	}

	return m_ReturnValue;
}

void App::DrawFrame()
{


}
