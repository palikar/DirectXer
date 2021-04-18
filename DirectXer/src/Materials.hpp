#pragma once

#include "Graphics.hpp"
#include "GraphicsCommon.hpp"
#include "Glm.hpp"

#include <type_traits>


struct TexturedMaterial
{
	ShaderConfig config;
	ConstantBufferId data;
	TextureId BaseMap;
	TextureId AoMap;
	TextureId EnvMap;
};

struct TexturedMaterialData
{
	glm::vec4 Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	float ColorIntensity{ 0.0f };
	float AoIntensity{ 0.0f };
	float Reflectivity{ 0.0f };
	float Refraction_ration{ 0.0 };
};

struct PhongMaterial
{
	ShaderConfig config;
	ConstantBufferId data;
};

struct PhongMaterialData
{
	glm::vec4 Ambient{ 0.0f, 0.0f, 0.0f, 0.0f};
	glm::vec4 Diffuse{ 0.0f, 0.0f, 0.0f, 0.0f};
	glm::vec4 Specular{ 0.0f, 0.0f, 0.0f, 0.0f};
	glm::vec4 Emissive{ 0.0f, 0.0f, 0.0f, 0.0f};
	float SpecularChininess {1.0};
	glm::vec3 _padding;
};
