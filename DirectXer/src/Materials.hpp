#pragma once

#include "Graphics.hpp"
#include "GraphicsCommon.hpp"
#include "Glm.hpp"

#include <type_traits>


template<typename MatDataType>
struct Material
{
	ShaderConfig config;
	CBObject cbBufferId;
	MatDataType data;
};

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

struct PhongMaterial
{
	ShaderConfig config;
	CBObject data;
};

struct PhongMaterialData
{
	glm::vec4 Ambient{ 0.0f, 0.0f, 0.0f, 0.0f};
	glm::vec4 Diffuse{ 0.0f, 0.0f, 0.0f, 0.0f};
	glm::vec4 Specular{ 0.0f, 0.0f, 0.0f, 0.0f};
	glm::vec4 Emissive{ 0.0f, 0.0f, 0.0f, 0.0f};
};
