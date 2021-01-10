#pragma once

#include "Geometry.hpp"
#include "IncludeWin.hpp"
#include "Graphics.hpp"

struct CommandLineSettings
{
};

struct Camera
{
	glm::vec3 Pos{0.0, 0.0, 0.0};
	glm::quat Rot{0.0, 0.0, 0.0, 1.0};


	inline void rotate(float t_Amount, glm::vec3  t_Axis)
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

class App
{
public:

	void Init(HWND t_Window);
	void Spin();

	void Destroy();

  private:
	Graphics m_Graphics;

	std::vector<float> plainGeometrVert;
	std::vector<uint32> plainGeometryInd;

	GeometryInfo cube;
	GeometryInfo plane;
	GeometryInfo sphere;

	Camera camera;

  public:
	int ReturnValue{0};
	boolean Running{true};

	
	
};
