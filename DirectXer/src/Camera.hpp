#pragma once

#include <Glm.hpp>
#include <Math.hpp>
#include <Input.hpp>

struct Camera
{
	glm::vec3 Pos{0.0, 0.0, 0.0};
	glm::quat Rot{0.0, 0.0, 0.0, 1.0};

	inline void rotate(float t_Amount, glm::vec3 t_Axis)
	{
		Rot *= glm::angleAxis(t_Amount, t_Axis);
	}

	inline void lookAt(glm::vec3 t_Point)
	{
		Rot = glm::angleAxis(glm::radians(180.0f), glm::normalize(t_Point - Pos));
	}

	inline glm::mat4 view()
	{
		return glm::lookAt(Pos, Pos + glm::axis(Rot), glm::vec3(0.0, 1.0, 0.0));
	}

	inline void move(float t_Amount, glm::vec3 t_Dir)
	{
		Pos += t_Dir*t_Amount;
	}

	inline glm::vec3 at()
	{
		return forward(Rot);
	}

	glm::vec3 up()
    {
        return glm::vec3(0, 1, 0);
    }

	inline glm::vec3 left()
	{
		return -glm::normalize(glm::cross(at(), up()));
	}
};

static void ControlCameraFPS(Camera& t_Camera, float dt = 1.0f)
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

static void ControlCameraOrbital(Camera& t_Camera, float dt = 1.0f)
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
			orbit += diff;
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
		radius = clamp(radius, 0.05f, 5.0f);
		update();
	}
	else if(scrollDelta < 0.0f)
	{
		radius += 0.15f;
		radius = clamp(radius, 0.05f, 3.0f);
		update();
	}
	
}

