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


using MaterialId = uint16;

struct TexturedMaterialData
{
	glm::vec4 Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	float ColorIntensity{ 0.0f };
	float AoIntensity{ 0.0f };
	float Reflectivity{ 0.0f };
	float Refraction_ration{ 0.0 };
};
// @Note: Basci textured material that supports color and AO maps, does not receive
// lightning, and can mix the texture color with some solid color
struct TexturedMaterial : public TexturedMaterialData
{
	ShaderConfiguration Program;

	ConstantBufferId Cbo;
	
	TextureId BaseMap;
	TextureId AoMap;
	TextureId EnvMap;
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

// @Note: Phong material that is pure color, implements the phong shading model
// and can receive lightning information
struct PhongMaterial : public PhongMaterialData
{
	ShaderConfiguration Program;
	
	ConstantBufferId Cbo;
};

#define KA_TEX_MASK  0x00100;
#define KA_FACT_MASK 0x00200;

#define KD_TEX_MASK  0x00400;
#define KD_FACT_MASK 0x00800;

#define KS_TEX_MASK  0x01000;
#define KS_FACT_MASK 0x02000;

#define NS_TEX_MASK  0x04000;
#define NS_FACT_MASK 0x08000;

#define D_TEX_MASK   0x10000;
#define D_FACT_MASK  0x20000;

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
	ShaderConfiguration Program;

	ConstantBufferId Cbo;
	
	TextureId KaMap;
	TextureId KdMap;
	TextureId KsMap;
	TextureId NsMap;
	TextureId dMap;
};


class MaterialLibrary
{
	struct MaterialProxy
	{
		uint16 Index;
		ConstantBufferId Cbo; 
		void* Data;
		uint32 DataSize;
	};
	
  public:
	BulkVector<MtlMaterial> MtlMaterials;
	BulkVector<PhongMaterial> PhongMaterials;
	BulkVector<TexturedMaterial> TexMaterials;

	Map<MaterialId, MaterialProxy> MaterialsIndex;


  public:

	void Init()
	{
		MtlMaterials.reserve(16);
		PhongMaterials.reserve(16);
		TexMaterials.reserve(16);

		MaterialsIndex.reserve(16 * 3);
	}
	
	void PutMaterial(MaterialId id, MtlMaterial mat)
	{
		MtlMaterials.push_back(mat);

		MaterialProxy newMaterial;
		newMaterial.Index = (uint16)MtlMaterials.size() - 1;
		newMaterial.Data = &MtlMaterials[newMaterial.Index - 1];
		newMaterial.DataSize = (uint32)sizeof(MtlMaterial);
		newMaterial.Cbo = mat.Cbo;

		MaterialsIndex.insert({id, newMaterial});
	}
	
	void PutMaterial(MaterialId id, PhongMaterial mat)
	{
		PhongMaterials.push_back(mat);

		MaterialProxy newMaterial;
		newMaterial.Index = (uint16)PhongMaterials.size() - 1;
		newMaterial.Data = &PhongMaterials[newMaterial.Index - 1];
		newMaterial.DataSize = (uint32)sizeof(PhongMaterial);
		newMaterial.Cbo = mat.Cbo;

		MaterialsIndex.insert({id, newMaterial});
	}
	
	void PutMaterial(MaterialId id, TexturedMaterial mat)
	{
		TexMaterials.push_back(mat);

		MaterialProxy newMaterial;
		newMaterial.Index = (uint16)TexMaterials.size() - 1;
		newMaterial.Data = &TexMaterials[newMaterial.Index - 1];
		newMaterial.DataSize = (uint32)sizeof(TexturedMaterial);
		newMaterial.Cbo = mat.Cbo;

		MaterialsIndex.insert({id, newMaterial});
	}
	
	void Update(Graphics* graphics, MaterialId id)
	{
		MaterialProxy mat = MaterialsIndex.at(id);
		graphics->UpdateCBs(mat.Cbo, mat.DataSize, mat.Data);
	}

	
	void UpdateAll(Graphics* graphics)
	{
		for (auto& [id, mat] : MaterialsIndex)
		{
			graphics->UpdateCBs(mat.Cbo, mat.DataSize, mat.Data);
		}
	}

	ConstantBufferId GetCbo(MaterialId id)
	{
		return MaterialsIndex.at(id).Cbo;
	}
		

	PhongMaterialData* GetPhongData(MaterialId id)
	{
		return (PhongMaterialData*)MaterialsIndex.at(id).Data;
	}
	
	TexturedMaterial* GetTexturedData(MaterialId id)
	{
		return (TexturedMaterial*)MaterialsIndex.at(id).Data;
	}
	
	MtlMaterialData* GetMtlData(MaterialId id)
	{
		return (MtlMaterialData*)MaterialsIndex.at(id).Data;	
	}

};
