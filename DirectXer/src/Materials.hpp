#pragma once

#include "Graphics.hpp"
#include "GraphicsCommon.hpp"
#include "Glm.hpp"

struct TexturedMaterial
{
	ShaderConfig config;
	CBObject data;
	TextureObject BaseMap;
	TextureObject AoMap;
	TextureObject EnvMap;
};

struct TexturedMaterialData
{
	glm::vec4 Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	float ColorIntensity{ 0.0f };
	float AoIntensity{ 0.0f };
	float Reflectivity{ 0.0f };
	float Refraction_ration{ 0.0 };
	
};
