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

enum CameraControlMode
{
	CCM_FPS,
	CCM_Orbital,
	CCM_Count,
};

struct CameraControlState
{
	bool InitialClick{false};
	glm::vec2 Look{0,0};
	glm::vec2 Orbit{0.0f, 0.0f};
	float Radius{1.0f};

	float MaxRadius{10.0f};
	float MinRadius{0.05f};

	int FastSpeed{15};
	int SlowSpeed{1};

	CameraControlMode Mode{CCM_FPS};
};

void ControlCameraFPS(CameraControlState& state, Camera& t_Camera, float dt = 1.0f);
void ControlCameraOrbital(CameraControlState& state, Camera& t_Camera, float dt = 1.0f);
void ControlCameraStateImgui(CameraControlState& state);

inline void ControlCamera(CameraControlState& state, Camera& t_Camera, float dt = 1.0f)
{
	switch (state.Mode) {
	  case CCM_FPS: 
		  ControlCameraFPS(state, t_Camera, dt);
		  break;
	 case CCM_Orbital:
		 ControlCameraOrbital(state, t_Camera, dt);
		 break;
	}
}

