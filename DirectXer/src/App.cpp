
#include "App.hpp"

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
		return dx::XMVectorSet(
			2*(m_Rot.x*m_Rot.z + m_Rot.w * m_Rot.y),
			2*(m_Rot.y*m_Rot.z - m_Rot.w * m_Rot.x),
			1 - 2*(m_Rot.x*m_Rot.x + m_Rot.y * m_Rot.y),
		0.0);
	}

	dx::XMVECTOR left()
	{
		return dx::XMVectorSet(
			1 - 2*(m_Rot.x * m_Rot.x + m_Rot.z * m_Rot.z),
			2*(m_Rot.x * m_Rot.y + m_Rot.w * m_Rot.z),
			2*(m_Rot.x * m_Rot.z - m_Rot.w * m_Rot.y),
		0.0);
	}

	

};

static void cameraController(Camera& t_Camera)
{
	
	// if (t_Win.m_Keyboard.KeyIsPressed(0x57))
	// {
	// 	auto at = t_Camera.at();
	// 	auto newPos = dx::XMVectorAdd(dx::XMLoadFloat3(&t_Camera.m_Pos), dx::XMVectorScale(t_Camera.at(), 0.01f));
	// 	dx::XMStoreFloat3(&t_Camera.m_Pos, newPos);
	// }

	// if (t_Win.m_Keyboard.KeyIsPressed(0x53))
	// {
	// 	auto at = t_Camera.at();
	// 	auto newPos = dx::XMVectorAdd(dx::XMLoadFloat3(&t_Camera.m_Pos), dx::XMVectorScale(t_Camera.at(), -0.01f));
	// 	dx::XMStoreFloat3(&t_Camera.m_Pos, newPos);
	// }

	// if (t_Win.m_Keyboard.KeyIsPressed(0x41))
	// {
	// 	auto at = t_Camera.at();
	// 	auto newPos = dx::XMVectorAdd(dx::XMLoadFloat3(&t_Camera.m_Pos), dx::XMVectorScale(t_Camera.left(), -0.01f));
	// 	dx::XMStoreFloat3(&t_Camera.m_Pos, newPos);
	// }

		
	// if (t_Win.m_Keyboard.KeyIsPressed(0x44))
	// {
	// 	auto at = t_Camera.at();
	// 	auto newPos = dx::XMVectorAdd(dx::XMLoadFloat3(&t_Camera.m_Pos), dx::XMVectorScale(t_Camera.left(), 0.01f));
	// 	dx::XMStoreFloat3(&t_Camera.m_Pos, newPos);
	// }

	// static bool initialClick = false;
	// static std::pair<int,int> lastPos = {};
	// static dx::XMVECTOR look{0,0};
		
	// if (t_Win.m_Mouse.LeftIsPressed())
	// {
	// 	if (!initialClick)
	// 	{
	// 		initialClick = true;
	// 		lastPos = t_Win.m_Mouse.GetPos();

	// 		return;
	// 	}
	// 	else
	// 	{
	// 		auto [xPos, yPos] = t_Win.m_Mouse.GetPos();
		
	// 		auto diff_x = (lastPos.first - xPos)*0.01f;
	// 		auto diff_y = (lastPos.second - yPos)*0.01f;
	// 		look = dx::XMVectorAdd(look, dx::XMVECTOR{ (float)diff_x, (float)diff_y });

	// 		float x = dx::XMVectorGetX(look);
	// 		float y = dx::XMVectorGetY(look);

	// 		if (y < -1.57)
	// 		{
	// 			y = -1.57;
	// 		}

	// 		if (y > 1.57)
	// 		{
	// 			y = 1.57;
	// 		}
			
	// 		if (x > 2 * 3.14)
	// 		{
	// 			x -= 2 * 3.14f;
	// 		}

	// 		if (x < -2 * 3.14)
	// 		{
	// 			x += 2 * 3.14f;
	// 		}

	// 		const float camX = 2.0f * sin(x);
	// 		const float camY = 1;
	// 		const float camZ = 2.0f * cos(x);

	// 		dx::XMFLOAT3 point;
	// 		dx::XMStoreFloat3(&point, dx::XMVectorAdd(dx::XMVectorSet(camX, camY, camZ, 0), dx::XMLoadFloat3(&t_Camera.m_Pos)));
	// 		t_Camera.lookAt(point);
	// 	}
	// }
	// else
	// {
	// 	initialClick = false;
	// }
	// lastPos = t_Win.m_Mouse.GetPos();



}

void App::Init(HWND t_Window)
{
	
	m_Graphics.initSwapChain(t_Window);
	m_Graphics.initBackBuffer();
	m_Graphics.initZBuffer();
	
	m_Graphics.initResources();

	
	cube = CubeGeometryInfo({});
	plane = PlaneGeometryInfo({1, 1, 5.0, 5.0});
	sphere = SphereGeometryInfo({2});
	
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

}

void App::Spin()
{


	Camera camera;
	camera.m_Pos = {0.0f, 0.0f, -0.5f};
    //cameraController(m_Window, camera);

	m_Graphics.ClearBuffer(0.0f, 0.0f, 0.0f);
	m_Graphics.ClearZBuffer();


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
	
	
}

void App::Destroy()
{
	m_Graphics.Destroy();
}
