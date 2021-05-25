#pragma once

#include <GraphicsCommon.hpp>
#include <Graphics.hpp>
#include <Glm.hpp>

static inline const uint8 SpotLightsCount = 5;
static inline const uint8 PointLightsCount = 5;

struct PointLight
{
	glm::vec4 Color{0.4f, 0.4f, 0.4f, 0.0f};
	glm::vec4 Position;
	glm::vec4 Params{0.0f, 0.5f, 0.2f, 0.0f};
	uint64 Active{0};
	uint64 _padding;
};

struct SpotLight
{
	
	glm::vec4 color{ 0.0f, 0.0f, 0.0f, 0.0f };
    glm::vec4 position{ 0.0f, 0.0f, 0.0f, 0.0f };
    glm::vec4 dir{ 0.0f, 0.0f, 0.0f, 0.0f};
    glm::vec4 Params{ 0.0f, 0.0f, 0.0f, 0.0f};
    uint64 Active{0};
	uint64 _padding;
};

struct Lighting
{
	glm::vec4 ambLightColor{0.0f, 0.0f, 0.0f, 1.0f};

	glm::vec4 dirLightColor{0.0f, 0.0f, 0.0f, 1.0f};
	glm::vec4 dirLightDir{0.5, 0.5f, 0, 0};

	PointLight pointLights[PointLightsCount];
	SpotLight  spotLights[SpotLightsCount];
	
};

struct LightSetup
{
	Lighting LightingData;
	ConstantBufferId Cbo;
};


bool ControlLightingImGui(Lighting& light);
