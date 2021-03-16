#pragma once

#include "Glm.hpp"
#include "Input.hpp"

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

	if (gInput.IsKeyPressed(KeyCode::W))
	{
		t_Camera.Pos = t_Camera.Pos + t_Camera.at() * 1.0f * dt;
	}

	if (gInput.IsKeyPressed(KeyCode::S))
	{
		t_Camera.Pos = t_Camera.Pos + t_Camera.at() * -1.0f * dt;
	}

	if (gInput.IsKeyPressed(KeyCode::A))
	{
		t_Camera.Pos = t_Camera.Pos + t_Camera.left() * 1.0f * dt;
	}
	
	if (gInput.IsKeyPressed(KeyCode::D))
	{
		t_Camera.Pos = t_Camera.Pos + t_Camera.left() * -1.0f * dt;
	}
		
	if (gInput.IsKeyPressed(KeyCode::E))
	{
		t_Camera.Pos = t_Camera.Pos + t_Camera.up() * 1.0f * dt;
	}

	if (gInput.IsKeyPressed(KeyCode::Q))
	{
		t_Camera.Pos = t_Camera.Pos + t_Camera.up() * -1.0f * dt;
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
			const auto diff = (gInput.MousePosition - gInput.LastMousePosition) * 0.8f * dt;
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

