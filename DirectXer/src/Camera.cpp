#include <imgui.h>

#include "Camera.hpp"

void ControlCameraFPS(CameraControlState& state, Camera& t_Camera, float dt)
{
	const float highSpeed = (float)state.FastSpeed;
	const float lowSpeed = (float)state.SlowSpeed;
	
	if (Input::gInput.IsKeyPressed(KeyCode::W))
	{
		const float speed = Input::gInput.IsKeyPressed(KeyCode::LeftShift) ? highSpeed : lowSpeed;
		t_Camera.Pos = t_Camera.Pos + t_Camera.at() * speed * dt;
	}

	if (Input::gInput.IsKeyPressed(KeyCode::S))
	{
		const float speed = Input::gInput.IsKeyPressed(KeyCode::LeftShift) ? highSpeed : lowSpeed;
		t_Camera.Pos = t_Camera.Pos + t_Camera.at() * -1.0f * dt * speed;
	}

	if (Input::gInput.IsKeyPressed(KeyCode::A))
	{
		const float speed = Input::gInput.IsKeyPressed(KeyCode::LeftShift) ? highSpeed : lowSpeed;
		t_Camera.Pos = t_Camera.Pos + t_Camera.left() * 1.0f * dt * speed;
	}
	
	if (Input::gInput.IsKeyPressed(KeyCode::D))
	{
		const float speed = Input::gInput.IsKeyPressed(KeyCode::LeftShift) ? highSpeed : lowSpeed;
		t_Camera.Pos = t_Camera.Pos + t_Camera.left() * -1.0f * dt * speed;
	}
		
	if (Input::gInput.IsKeyPressed(KeyCode::E))
	{
		const float speed = Input::gInput.IsKeyPressed(KeyCode::LeftShift) ? highSpeed : lowSpeed;
		t_Camera.Pos = t_Camera.Pos + t_Camera.up() * 1.0f * dt * speed;
	}

	if (Input::gInput.IsKeyPressed(KeyCode::Q))
	{
		const float speed = Input::gInput.IsKeyPressed(KeyCode::LeftShift) ? highSpeed : lowSpeed;
		t_Camera.Pos = t_Camera.Pos + t_Camera.up() * -1.0f * dt * speed;
	}

	auto& initialClick = state.InitialClick;
	auto& look = state.Look;

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

void ControlCameraOrbital(CameraControlState& state, Camera& t_Camera, float)
{
	auto& initialClick = state.InitialClick;
	auto& orbit = state.Orbit;
	auto& radius = state.Radius;

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
		radius = clamp(radius, state.MinRadius, state.MaxRadius);
		update();
	}
	else if(scrollDelta < 0.0f)
	{
		radius += 0.15f;
		radius = clamp(radius, state.MinRadius, state.MaxRadius);
		update();
	}
	
}

void ControlCameraStateImgui(CameraControlState& state)
{
	if (ImGui::CollapsingHeader("Camera Control"))
	{

		const char* elems_names[CCM_Count] = { "FPS", "Orbital" };
		const char* elem_name = (state.Mode >= 0 && state.Mode < CCM_Count) ? elems_names[state.Mode] : "Unknown";
		ImGui::SliderInt("Control Mode", (int*)&state.Mode, 0, CCM_Count - 1, elem_name);
		ImGui::Separator();

		if (state.Mode == CCM_FPS)
		{
			ImGui::Text("FPS camera:");
			ImGui::SliderInt("Slow fly speed", &state.SlowSpeed, 1, 15, "%d");
			ImGui::SliderInt("Fast fly speed", &state.FastSpeed, 10, 55, "%d");			
		}
		else if (state.Mode == CCM_Orbital)
		{
			ImGui::Text("Orbital camera:");
			ImGui::InputFloat("Max Radius", &state.MaxRadius, 1.0f, 5.0f);
			ImGui::InputFloat("Min Radius", &state.MinRadius, 1.0f, 5.0f);
		}		
	}
}
