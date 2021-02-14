#pragma once

#include "GraphicsCommon.hpp"
#include "Graphics.hpp"
#include "Glm.hpp"

struct Lighting
{
	glm::vec4 ambLightColor{0.0f, 0.0f, 0.0f, 1.0f};

	glm::vec4 dirLightColor{0.0f, 0.0f, 0.0f, 1.0f};
	glm::vec4 dirLightDir{0.5, 0.5f, 0, 0};
};


struct LightSetup
{
	Lighting lighting;
	CBObject bufferId;
};
