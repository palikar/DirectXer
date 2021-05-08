#pragma once

#include <Graphics.hpp>
#include <GraphicsCommon.hpp>
#include <Glm.hpp>

/* 
   @Note: "Material" is nothing more than a shader and some data
   accosiated with the execution of the shaer; in DirectXer we follow
   a minimalist approach to materials where they are just POD;
   "activating" a material consists of binding some GPU objects in the
   pipeline and updating some constant buffer with the material's data
 
   @Todo: It is currently unclear what belongs in the data, and what belongs
   in the material structure; for example, do we want the textures to be part
   of the data (and hence be part of the "material instance" as it may be called
   in other engines\material systems) or do we want them to be part of the material
   itself and have some hard dependency between the shader progran and the textures
   that the material uses.

   For now, the strucutres reside in the material and each material is define through
   the textures it is using.
 
*/


using MaterialId = uint32;

// @Note: Basci textured material that supports color and AO maps, does not receive
// lightning, and can mix the texture color with some solid color
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

// @Note: Phong material that is pure color, implements the phong shading model
// and can receive lightning information
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

// @Note: Loaded from Mtl file;
struct MtlMaterialData
{
	float Ns;
	glm::vec3 Ka;

	float Ni;
	glm::vec3 Kd;

	float d;
	glm::vec3 Ks;

	glm::vec3 Ke;
	uint32 illum;
};

struct MtlMaterial : public MtlMaterialData
{
	ShaderConfig Program;

	ConstantBufferId Cbo;
	
	TextureId KaMap;
	TextureId KdMap;
	TextureId KsMap;
	TextureId NsMap;
	TextureId dMap;
};
