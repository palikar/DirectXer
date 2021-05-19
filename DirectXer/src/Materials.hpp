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

   @Amendment 1: It turned out that it is usefull to think of the whole material as
   one single thing in all context; for this reason the material data now is a base
   class of the material itself; this relationship is to be though as "has a" instead
   of the classical "is a" thing; so, the material X *has* a material X-data; with thise
   setup, a meterial can be created and its adress can be used as a data pointer to
   update the GPU constant buffer
 
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
	MaterialId Id;
	
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
	MaterialId Id;
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
	// @Note: Ambient factors
	glm::vec3 Ka;

	float Ni;
	// @Note: Diffuse factors
	glm::vec3 Kd;

	float d;
	// @Note: Specular factors
	glm::vec3 Ks;

	// @Note: Specular exponents
	glm::vec3 Ke;

	// @Note: The lowest bits of this hold the illumination models;
	// the higher bits (see the defines above) contain information
	// what maps (textures) are used for which properties
	uint32 illum;
};

struct MtlMaterial : public MtlMaterialData
{
	ShaderConfiguration Program;
	ConstantBufferId Cbo;
	MaterialId Id;
	
	TextureId KaMap;
	TextureId KdMap;
	TextureId KsMap;
	TextureId NsMap;
	TextureId dMap;
};


class MaterialLibrary
{
	struct MaterialUpdateProxy
	{
		ConstantBufferId Cbo; 
		void* Data;
		uint32 DataSize;
	};

	struct MaterialBindProxy
	{
		ShaderConfiguration Program;
		ConstantBufferId Cbo;
		TextureId* Textures[5];
	}
	
  public:
	BulkVector<MtlMaterial> MtlMaterials;
	BulkVector<PhongMaterial> PhongMaterials;
	BulkVector<TexturedMaterial> TexMaterials;

	Map<MaterialId, MaterialUpdateProxy> UpdateViews;
	Map<MaterialId, MaterialBindProxy> BindViews;

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
	}
	
	void PutMaterial(MaterialId id, PhongMaterial mat)
	{
		PhongMaterials.push_back(mat);
	}
	
	void PutMaterial(MaterialId id, TexturedMaterial mat)
	{
		TexMaterials.push_back(mat);
	}

	void GenerateProxies()
	{
		for (auto& mat : MtlMaterials)
		{
			MaterialBindProxy bind {0};
			bind.Cbo = mat.Cbo;
			bind.Program = mat.Program;
			bind.Textures[0] = &mat.KaMap;
			bind.Textures[1] = &mat.KdMap;
			bind.Textures[2] = &mat.KsMap;
			bind.Textures[3] = &mat.NsMap;
			bind.Textures[4] = &mat.dMap;
			BindViews.insert({mat.Id, bind});

			MaterialUpdateProxy upadte;
			upadte.Cbo = mat.Cbo;
			upadte.Data = &mat;
			upadte.DataSize = sizeof(MtlMaterial);
			UpdateViews.insert({mat.Id, upadte});			
		}

		for (auto& mat : PhongMaterials)
		{
			MaterialBindProxy bind {0};
			bind.Cbo = mat.Cbo;
			bind.Program = mat.Program;
			BindViews.insert({mat.Id, bind});

			MaterialUpdateProxy upadte;
			upadte.Cbo = mat.Cbo;
			upadte.Data = &mat;
			upadte.DataSize = sizeof(MtlMaterial);
			UpdateViews.insert({mat.Id, upadte});			
		}

		for (auto& mat : TexturedMaterials)
		{
			MaterialBindProxy bind {0};
			bind.Cbo = mat.Cbo;
			bind.Program = mat.Program;
			bind.Textures[0] = &mat.BaseMap;
			bind.Textures[1] = &mat.AoMap;
			bind.Textures[2] = &mat.EnvMap;
			BindViews.insert({mat.Id, bind});

			MaterialUpdateProxy upadte;
			upadte.Cbo = mat.Cbo;
			upadte.Data = &mat;
			upadte.DataSize = sizeof(MtlMaterial);
			UpdateViews.insert({mat.Id, upadte});			
		}

	}

	
	void Update(Graphics* graphics, MaterialId id)
	{
		MaterialUpdateProxy mat = MaterialsIndex.at(id);
		graphics->UpdateCBs(mat.Cbo, mat.DataSize, mat.Data);
	}
	
	void UpdateAll(Graphics* graphics)
	{
		for (auto& [id, mat] : UpdateViews)
		{
			graphics->UpdateCBs(mat.Cbo, mat.DataSize, mat.Data);
		}
	}


	void Bind(Graphics* graphics, MaterialId id)
	{
		MaterialBindProxy mat = BindViews.at(id);
		graphics->SetShaderConfiguration(mat.Program);
		graphics->BindVSConstantBuffers(mat.Cbo, 1);

		for (uint32 i = 0; i < 5; ++i)
		{
			if (mat.Textures[i]) graphics->BindVSTexture(i + 1, *mat.Textures[i]);	
		}
		
	}
		

	PhongMaterialData* GetPhongData(MaterialId id)
	{
		return (PhongMaterialData*)UpdateViews.at(id).Data;
	}
	
	TexturedMaterial* GetTexturedData(MaterialId id)
	{
		return (TexturedMaterial*)UpdateViews.at(id).Data;
	}
	
	MtlMaterialData* GetMtlData(MaterialId id)
	{
		return (MtlMaterialData*)UpdateViews.at(id).Data;	
	}

	
	PhongMaterial& GetPhong(MaterialId id)
	{
		return std::find(PhongMaterials.begin(), PhongMaterials.end(), [id](auto& m) {
			m.Id == id;
		});
	}
	
	TexturedMaterial& GetTextured(MaterialId id)
	{
		return std::find(TexutredMaterials.begin(), TexutredMaterials.end(), [id](auto& m) {
			m.Id == id;
		});
	}
	
	MtlMaterial& GetMtl(MaterialId id)
	{
		return std::find(MtlMaterials.begin(), MtlMaterials.end(), [id](auto& m) {
			m.Id == id;
		});
	}

};

void InitMaterial(Graphics* graphics, PhongMaterial& mat, String debugName )
{
	mat.Program = SC_DEBUG_TEX;
	mat.Cbo = NextConstantBufferId();
	Graphics->CreateConstantBuffer(mat.Cbo, sizeof(PhongdMaterialData), &mat);
	Graphics->SetConstantBufferName(texMat.Cbo, debugName);
}

void InitMaterial(Graphics* graphics, TexturedMaterial& mat, String debugName)
{
	mat.Program = SC_DEBUG_TEX;
	mat.Cbo = NextConstantBufferId();
	Graphics->CreateConstantBuffer(mat.Cbo, sizeof(TexturedMaterialData), &mat);
	Graphics->SetConstantBufferName(texMat.Cbo, debugName);
}


bool ControlPhongMaterial(MaterialId id, char* name)
{


}
