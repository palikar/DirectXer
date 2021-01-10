#include "App.hpp"
#include "Glm.hpp"
#include "Input.hpp"
#include "Logging.hpp"

#include <iostream>


static void cameraController(Camera& t_Camera)
{

	if (gInput.IsKeyPressed(KeyCode::W))
	{
		t_Camera.Pos = t_Camera.Pos + t_Camera.at() * 0.01f;
	}

	if (gInput.IsKeyPressed(KeyCode::S))
	{
		t_Camera.Pos = t_Camera.Pos + t_Camera.at() * -0.01f;
	}

	if (gInput.IsKeyPressed(KeyCode::A))
	{
		t_Camera.Pos = t_Camera.Pos + t_Camera.left() * 0.01f;
	}
	
	if (gInput.IsKeyPressed(KeyCode::D))
	{
		t_Camera.Pos = t_Camera.Pos + t_Camera.left() * -0.01f;
	}



	static bool initialClick = false;
	static glm::vec2 look{0,0};

	if (gInput.IsKeyPressed(MouseCode::BUTTON_2))
	{
		if (!initialClick)
		{
			initialClick = true;
			return;
		}
		else
		{
			const auto diff = (gInput.MousePosition - gInput.LastMousePosition) * 0.01f;
			look += diff;

			float x = -look.x;
			float y = look.y;

			if (y < -1.57)
			{
				y = -1.57;
			}

			if (y > 1.57)
			{
				y = 1.57;
			}

			if (x > 2 * 3.14)
			{
				x -= 2 * 3.14f;
			}

			if (x < -2 * 3.14)
			{
				x += 2 * 3.14f;
			}

			const float camX = -2.0f * sin(x) * cos(y);
			const float camY = -2.0f * sin(y);
			const float camZ = -2.0f * cos(x) * cos(y);
			
			t_Camera.lookAt(glm::vec3{camX, camY, camZ} + t_Camera.Pos);
		}
	}
	else
	{
		initialClick = false;
	}


}

void App::Init(HWND t_Window)
{

	Graphics.initSwapChain(t_Window, Width, Height);
	Graphics.initBackBuffer();
	Graphics.initZBuffer(Width, Height);

	Graphics.initResources();


	cube = CubeGeometryInfo({});
	plane = PlaneGeometryInfo({1, 1, 5.0, 5.0});
	sphere = SphereGeometryInfo({2});

	CubeGeometry({}, plainGeometrVert, plainGeometryInd);
	PlaneGeometry({1, 1, 5.0, 5.0}, plainGeometrVert, plainGeometryInd);
	SphereGeometry({2}, plainGeometrVert, plainGeometryInd);

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

	cameraController(camera);

	Graphics.ClearBuffer(0.0f, 0.0f, 0.0f);
	Graphics.ClearZBuffer();

	Graphics.m_VertexShaderCB.projection = glm::transpose(glm::perspective(65.0f, 3.0f/4.0f, 0.0001f, 1000.0f));
	Graphics.m_VertexShaderCB.view = glm::transpose(camera.view());

	Graphics.m_PixelShaderCB.color = { 0.0, 1.0, 1.0, 1.0 };
	Graphics.m_VertexShaderCB.model = glm::mat4(1);

	Graphics.updateCBs();
	Graphics.drawIndex(Graphics::TT_TRIANGLES, cube.indexCount);

	Graphics.m_PixelShaderCB.color = { 1.0, 0.0, 0.0, 1.0};
	Graphics.m_VertexShaderCB.model = init_translate(0.5, 0.0, 0.0);

	Graphics.updateCBs();
	Graphics.drawIndex(Graphics::TT_TRIANGLES, plane.indexCount, cube.indexCount, cube.vertexCount/3);

	Graphics.m_PixelShaderCB.color = { 0.0, 1.0, 0.0, 1.0 };
	Graphics.m_VertexShaderCB.model = init_translate(3.0, 0.0, 3.0) * init_scale(0.3, 0.3, 0.3);

	Graphics.updateCBs();
	Graphics.drawIndex(Graphics::TT_TRIANGLES, sphere.indexCount, cube.indexCount + plane.indexCount, cube.vertexCount/3 + plane.vertexCount/3);

	Graphics.EndFrame();

}

void App::Destroy()
{
	Graphics.Destroy();
}
