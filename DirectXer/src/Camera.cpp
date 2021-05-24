

#include "Camera.hpp"


void ControlCameraFPS(Camera& t_Camera, float dt)
{
	const float highSpeed = 15.0f;
	
	if (Input::gInput.IsKeyPressed(KeyCode::W))
	{
		const float speed = Input::gInput.IsKeyPressed(KeyCode::LeftShift) ? highSpeed : 1.0f;
		t_Camera.Pos = t_Camera.Pos + t_Camera.at() * speed * dt;
	}

	if (Input::gInput.IsKeyPressed(KeyCode::S))
	{
		const float speed = Input::gInput.IsKeyPressed(KeyCode::LeftShift) ? highSpeed : 1.0f;
		t_Camera.Pos = t_Camera.Pos + t_Camera.at() * -1.0f * dt * speed;
	}

	if (Input::gInput.IsKeyPressed(KeyCode::A))
	{
		const float speed = Input::gInput.IsKeyPressed(KeyCode::LeftShift) ? highSpeed : 1.0f;
		t_Camera.Pos = t_Camera.Pos + t_Camera.left() * 1.0f * dt * speed;
	}
	
	if (Input::gInput.IsKeyPressed(KeyCode::D))
	{
		const float speed = Input::gInput.IsKeyPressed(KeyCode::LeftShift) ? highSpeed : 1.0f;
		t_Camera.Pos = t_Camera.Pos + t_Camera.left() * -1.0f * dt * speed;
	}
		
	if (Input::gInput.IsKeyPressed(KeyCode::E))
	{
		const float speed = Input::gInput.IsKeyPressed(KeyCode::LeftShift) ? highSpeed : 1.0f;
		t_Camera.Pos = t_Camera.Pos + t_Camera.up() * 1.0f * dt * speed;
	}

	if (Input::gInput.IsKeyPressed(KeyCode::Q))
	{
		const float speed = Input::gInput.IsKeyPressed(KeyCode::LeftShift) ? highSpeed : 1.0f;
		t_Camera.Pos = t_Camera.Pos + t_Camera.up() * -1.0f * dt * speed;
	}

	static bool initialClick = false;
	static glm::vec2 look{0,0};

	if (Input::gInput.IsKeyPressed(MouseCode::BUTTON_2))
	{
		if (!initialClick)
		{
			initialClick = true;
			return;
		}
		else
		{
			const auto diff = (Input::gInput.MousePosition - Input::gInput.LastMousePosition) * 0.01f;
			look += diff;

			float x = -look.x;
			float y = look.y;

			if (y < -1.57f)
			{
				y = -1.57f;
			}

			if (y > 1.57f)
			{
				y = 1.57f;
			}

			if (x > 2 * 3.14f)
			{
				x -= 2 * 3.14f;
			}

			if (x < -2 * 3.14f)
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

void ControlCameraOrbital(Camera& t_Camera, float dt)
{
	static bool initialClick = false;
	static glm::vec2 orbit{0.0f, 0.0f};
	static float radius = 1.0f;

	const auto update = [&]()
	{
		orbit.y = clamp(orbit.y, -1.57f, 1.57f);

		if (orbit.x > 2 * PI) orbit.x -= 2 * PI;
		if (orbit.x < -2 * PI) orbit.x += 2 * PI;

		const float camX = -radius * sin(orbit.x) * cos(orbit.y);
		const float camY = -radius * sin(orbit.y);
		const float camZ = -radius * cos(orbit.x) * cos(orbit.y);

		t_Camera.Pos = glm::vec3{ camX, camY, camZ };
		t_Camera.lookAt(glm::vec3(0.0f));
	};

	if (Input::gInput.IsKeyPressed(MouseCode::BUTTON_2))
	{
		if (!initialClick)
		{
			initialClick = true;
			return;
		}
		else
		{
			const auto diff = (Input::gInput.MousePosition - Input::gInput.LastMousePosition) * 0.01f;
			orbit += -diff;
			update();
		}
	}
	else
	{
		initialClick = false;
	}

	const float scrollDelta = Input::gInput.MouseScrollOffset.x;
	if (scrollDelta > 0.0f)
	{
		radius -= 0.15f;
		radius = clamp(radius, 0.05f, 10.0f);
		update();
	}
	else if(scrollDelta < 0.0f)
	{
		radius += 0.15f;
		radius = clamp(radius, 0.05f, 10.0f);
		update();
	}
	
}
