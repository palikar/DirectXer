#include "App.h"

#include "Geometry.hpp"
#include <iostream>


struct Camera
{
	dx::XMFLOAT3 m_Pos{0.0, 0.0, 0.0};
	dx::XMFLOAT4 m_Rot{0.0, 0.0, 0.0, 1.0};

	Camera()
	{
		const auto rot = dx::XMQuaternionRotationAxis(dx::XMVectorSet(0.0, 1.0, 0.0, 0.0), 0.0f);
		dx::XMStoreFloat4(&m_Rot, rot);
	}

	void move(float t_Amount, dx::XMFLOAT3 t_Dir)
	{
		dx::XMStoreFloat3(&m_Pos, dx::XMVectorScale(dx::XMLoadFloat3(&t_Dir),t_Amount));
	}

	void rotate(dx::XMFLOAT3 t_Rot)
	{
		const auto newRot = dx::XMQuaternionRotationRollPitchYawFromVector(dx::XMLoadFloat3(&t_Rot));
		dx::XMStoreFloat4(&m_Rot, newRot);
	}
	
	void rotate(float t_Amount, dx::XMFLOAT3 t_Axis)
	{
		const auto rot = dx::XMQuaternionRotationAxis(dx::XMLoadFloat3(&t_Axis), t_Amount);
		dx::XMStoreFloat4(&m_Rot, dx::XMQuaternionMultiply(rot, dx::XMLoadFloat4(&m_Rot)));
	} 

	void lookAt(dx::XMFLOAT3 t_Point)
	{
		const auto rot = dx::XMMatrixLookAtLH(dx::XMLoadFloat3(&m_Pos), dx::XMLoadFloat3(&t_Point), dx::XMVectorSet(0.0, 1.0, 0.0, 0.0));
		dx::XMStoreFloat4(&m_Rot, dx::XMQuaternionRotationMatrix(rot));
	}

	dx::XMMATRIX view()
	{
		dx::XMVECTOR pos = dx::XMLoadFloat3(&m_Pos);
		dx::XMVECTOR rot = dx::XMLoadFloat4(&m_Rot);		
		return (dx::XMMatrixTranslationFromVector(dx::XMVectorMultiply(pos, dx::XMVectorSet(-1.0, -1.0, -1.0, -1.0))) * dx::XMMatrixRotationQuaternion(rot));
	}

	dx::XMVECTOR at()
	{
		// dx::XMVECTOR axis;
		// float angle;
		// dx::XMQuaternionToAxisAngle(&axis, &angle, dx::XMLoadFloat4(&m_Rot));
		return dx::XMVectorSet(
			2*(m_Rot.x*m_Rot.z + m_Rot.w * m_Rot.y),
			2*(m_Rot.y*m_Rot.z - m_Rot.w * m_Rot.x),
			1 - 2*(m_Rot.x*m_Rot.x + m_Rot.y * m_Rot.y),
		0.0);
	}

	

};

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

	Camera camera;

	camera.m_Pos = {0.0f, 0.0f, -0.5f};

	float t = 0.0;
	while (m_Running)
	{
		processMessages();

		if (m_Window.m_Keyboard.KeyIsPressed(0x57))
		{
			OutputDebugString("pressed!\n");
			//auto newPos = dx::XMVectorAdd(dx::XMLoadFloat3(&camera.m_Pos), dx::XMVectorSet(0.0, 0.0, 0.01f, 0.0));
			auto at = camera.at();
			auto x = dx::XMVectorGetX(at);
			auto y = dx::XMVectorGetY(at);
			auto z = dx::XMVectorGetZ(at);

			auto newPos = dx::XMVectorAdd(dx::XMLoadFloat3(&camera.m_Pos), dx::XMVectorScale(camera.at(), 0.01f));
			dx::XMStoreFloat3(&camera.m_Pos, newPos);
		}


		m_Graphics.ClearBuffer(0.0f, 0.0f, 0.0f);
		m_Graphics.ClearZBuffer();

		//auto x = 5.5f * std::sinf(t);
		//auto y = 5.5f * std::cosf(t);

		//camera.m_Pos = {x, 5.5f, y};
		//camera.lookAt({0.0, 0.0, 0.0});

		m_Graphics.m_VertexShaderCB.projection = dx::XMMatrixTranspose(dx::XMMatrixPerspectiveFovLH(65.0f, 3.0f/4.0f, 0.0001, 1000.0));
		m_Graphics.m_VertexShaderCB.view = dx::XMMatrixTranspose(camera.view());			


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
