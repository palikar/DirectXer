#include "Materials.hpp"

#include <TextureCatalog.hpp>

#include <imgui.h>

void MaterialLibrary::Init()
{
	MtlMaterials.reserve(16);
	PhongMaterials.reserve(16);
	TexMaterials.reserve(16);

	UpdateViews.reserve(16 * 3);
	BindViews.reserve(16 * 3);
}

void MaterialLibrary::GenerateProxy(PhongMaterial& mat)
{
	MaterialBindProxy bind {};
	bind.Cbo = mat.Cbo;
	bind.Program = mat.Program;
	bind.BindToPS = true;
	bind.Slot = 1;
	BindViews.insert({mat.Id, bind});

	MaterialUpdateProxy upadte;
	upadte.Cbo = mat.Cbo;
	upadte.Data = &mat;
	upadte.DataSize = sizeof(PhongMaterialData);
	UpdateViews.insert({mat.Id, upadte});	
}

void MaterialLibrary::GenerateProxy(TexturedMaterial& mat)
{
	MaterialBindProxy bind {};
	bind.Cbo = mat.Cbo;
	bind.Program = mat.Program;
	bind.Textures[0] = &mat.BaseMap;
	bind.Textures[1] = &mat.AoMap;
	bind.Textures[2] = &mat.EnvMap;
	bind.BindToPS = true;
	bind.Slot = 1;
	BindViews.insert({mat.Id, bind});

	MaterialUpdateProxy upadte;
	upadte.Cbo = mat.Cbo;
	upadte.Data = &mat;
	upadte.DataSize = sizeof(TexturedMaterialData);
	UpdateViews.insert({mat.Id, upadte});	
}

void MaterialLibrary::GenerateProxy(MtlMaterial& mat)
{
	MaterialBindProxy bind {};
	bind.Cbo = mat.Cbo;
	bind.Program = mat.Program;
	bind.Textures[0] = &mat.KaMap;
	bind.Textures[1] = &mat.KdMap;
	bind.Textures[2] = &mat.KsMap;
	bind.Textures[3] = &mat.NsMap;
	bind.Textures[4] = &mat.dMap;
	bind.BindToPS = false;
	bind.Slot = 1;
	BindViews.insert({mat.Id, bind});

	MaterialUpdateProxy upadte;
	upadte.Cbo = mat.Cbo;
	upadte.Data = &mat;
	upadte.DataSize = sizeof(MtlMaterial);
	UpdateViews.insert({mat.Id, upadte});
	
}

void MaterialLibrary::GenerateProxies()
{
	for (auto& mat : MtlMaterials)
	{
		MaterialBindProxy bind {};
		bind.Cbo = mat.Cbo;
		bind.Program = mat.Program;
		bind.Textures[0] = &mat.KaMap;
		bind.Textures[1] = &mat.KdMap;
		bind.Textures[2] = &mat.KsMap;
		bind.Textures[3] = &mat.NsMap;
		bind.Textures[4] = &mat.dMap;
		bind.BindToPS = false;
		bind.Slot = 1;
		BindViews.insert({mat.Id, bind});

		MaterialUpdateProxy upadte;
		upadte.Cbo = mat.Cbo;
		upadte.Data = &mat;
		upadte.DataSize = sizeof(MtlMaterial);
		UpdateViews.insert({mat.Id, upadte});
	}

	for (auto& mat : PhongMaterials)
	{
		MaterialBindProxy bind {};
		bind.Cbo = mat.Cbo;
		bind.Program = mat.Program;
		bind.BindToPS = true;
		bind.Slot = 1;
		BindViews.insert({mat.Id, bind});

		MaterialUpdateProxy upadte;
		upadte.Cbo = mat.Cbo;
		upadte.Data = &mat;
		upadte.DataSize = sizeof(MtlMaterial);
		UpdateViews.insert({mat.Id, upadte});
	}

	for (auto& mat : TexMaterials)
	{
		MaterialBindProxy bind {};
		bind.Cbo = mat.Cbo;
		bind.Program = mat.Program;
		bind.Textures[0] = &mat.BaseMap;
		bind.Textures[1] = &mat.AoMap;
		bind.Textures[2] = &mat.EnvMap;
		bind.BindToPS = true;
		bind.Slot = 1;
		BindViews.insert({mat.Id, bind});

		MaterialUpdateProxy upadte;
		upadte.Cbo = mat.Cbo;
		upadte.Data = &mat;
		upadte.DataSize = sizeof(MtlMaterial);
		UpdateViews.insert({mat.Id, upadte});
	}

}

void MaterialLibrary::Update(Graphics* graphics, MaterialId id)
{
	MaterialUpdateProxy mat = UpdateViews.at(id);
	graphics->UpdateCBs(mat.Cbo, mat.DataSize, mat.Data);
}

void MaterialLibrary::UpdateAll(Graphics* graphics)
{
	for (auto& [id, mat] : UpdateViews)
	{
		graphics->UpdateCBs(mat.Cbo, mat.DataSize, mat.Data);
	}
}

void MaterialLibrary::Bind(Graphics* graphics, MaterialId id)
{
	MaterialBindProxy mat = BindViews.at(id);
	graphics->SetShaderConfiguration(mat.Program);

	if (mat.BindToPS)
	{
		graphics->BindPSConstantBuffers(mat.Cbo, mat.Slot);
		for (uint32 i = 0; i < 5; ++i)
		{
			if (mat.Textures[i]) graphics->BindTexture(i + 1, *mat.Textures[i]);
		}
	}
	else
	{
		graphics->BindVSConstantBuffers(mat.Cbo, mat.Slot);
		for (uint32 i = 0; i < 5; ++i)
		{
			if (mat.Textures[i] && *mat.Textures[i] != 0) graphics->BindVSTexture(i + 1, *mat.Textures[i]);
		}
	}
		
}

PhongMaterialData* MaterialLibrary::GetPhongData(MaterialId id)
{
	return (PhongMaterialData*)UpdateViews.at(id).Data;
}

TexturedMaterial* MaterialLibrary::GetTexturedData(MaterialId id)
{
	return (TexturedMaterial*)UpdateViews.at(id).Data;
}

MtlMaterialData* MaterialLibrary::GetMtlData(MaterialId id)
{
	return (MtlMaterialData*)UpdateViews.at(id).Data;
}

PhongMaterial& MaterialLibrary::GetPhong(MaterialId id)
{
	return *std::find_if(PhongMaterials.begin(), PhongMaterials.end(), [id](auto& m) {
		return m.Id == id;
	});
}

TexturedMaterial& MaterialLibrary::GetTextured(MaterialId id)
{
	return *std::find_if(TexMaterials.begin(), TexMaterials.end(), [id](auto& m) {
		return m.Id == id;
	});
}

MtlMaterial& MaterialLibrary::GetMtl(MaterialId id)
{
	return *std::find_if(MtlMaterials.begin(), MtlMaterials.end(), [id](auto& m) {
		return m.Id == id;
	});
}




static void ChooseTexture(Graphics* graphics, TextureCatalog& catalog, const char* title, TextureId& texId)
{
	auto it = std::find_if(catalog.LoadedTextures.begin(), catalog.LoadedTextures.end(), [&texId](auto& t) { return t.Handle == texId; });
	auto texName = it != catalog.LoadedTextures.end() ? it->Name.data() : "NotSelected";
	if (ImGui::BeginCombo(title, texName))
	{
		for (int i = 0; i < (int)catalog.LoadedTextures.size(); ++i)
		{
			auto& tex = catalog.LoadedTextures[i];
			if (ImGui::Selectable(tex.Name.data(), tex.Handle == texId))
			{
				texId = tex.Handle;
			}

		}
		ImGui::EndCombo();
	}
	if (texId != 0)
	{
		ImGui::SameLine();
		ImGui::Image(graphics->Textures.at(texId).srv, { 64, 64 });
	}

	


}

bool ControlMtlMaterialImGui(MtlMaterial& mat, const char* name, TextureCatalog& textures, Graphics* graphics)
{
	bool changed = false;
	bool hasKaMask = (mat.illum & KA_TEX_MASK ) > 0;
	bool hasKdMask = (mat.illum & KD_TEX_MASK ) > 0;
	bool hasKsMask = (mat.illum & KS_TEX_MASK ) > 0;
	bool hasNsMask = (mat.illum & NS_TEX_MASK ) > 0;
	bool hasDMask  = (mat.illum & D_TEX_MASK  )  > 0;

	ImGui::Text("Texture Masks");

	ImGui::Checkbox("Ka", &hasKaMask);
	ImGui::SameLine();
	if (hasKaMask) mat.illum |= KA_TEX_MASK;
	else  mat.illum &= ~KA_TEX_MASK;

	ImGui::Checkbox("Kd", &hasKdMask);
	ImGui::SameLine();
	if (hasKdMask) mat.illum |= KD_TEX_MASK;
	else  mat.illum &= ~KD_TEX_MASK;

	ImGui::Checkbox("Ks", &hasKsMask);
	ImGui::SameLine();
	if (hasKsMask) mat.illum |= KS_TEX_MASK;
	else  mat.illum &= ~KS_TEX_MASK;
	
	ImGui::Checkbox("Ns", &hasNsMask);
	ImGui::SameLine();
	if (hasNsMask) mat.illum |= NS_TEX_MASK;
	else  mat.illum &= ~NS_TEX_MASK;

	ImGui::Checkbox("D", &hasDMask);
	if (hasDMask) mat.illum |= D_TEX_MASK;
	else  mat.illum &= ~D_TEX_MASK;
		
	ImGui::Separator();
		
	ImGui::Text("Colors:");
	changed |= ImGui::ColorEdit3("Ambient Factors", (float*)&mat.Ka);
	changed |= ImGui::ColorEdit3("Diffuse Factors", (float*)&mat.Kd);
	changed |= ImGui::ColorEdit3("Specular Factors", (float*)&mat.Ks);
	changed |= ImGui::SliderFloat("Specular Exponent", (float*)&mat.Ns, 0.0f, 25.0f, "%.3f");

	ImGui::Separator();
	ImGui::Text("Mat Properties:");
	changed |= ImGui::SliderFloat("Optical Density", (float*)&mat.Ni, 0.0f, 25.0f, "%.3f");
	changed |= ImGui::SliderFloat("Dissolve Factor", (float*)&mat.d, 0.0f, 25.0f, "%.3f");

	ImGui::Separator();
	ImGui::Text("Maps: ");

	ChooseTexture(graphics, textures, "Ka Map", mat.KaMap);
	ChooseTexture(graphics, textures, "Kd Map", mat.KdMap);
	ChooseTexture(graphics, textures, "Ks Map", mat.KsMap);
	ChooseTexture(graphics, textures, "Ns Map", mat.NsMap);
	ChooseTexture(graphics, textures, "d Map", mat.dMap);
	

	return changed;
}

bool ControlTexturedMaterialImGui(TexturedMaterial& mat, const char* name, TextureCatalog& textures, Graphics* graphics)
{
	bool changed = false;
	changed |= ImGui::ColorEdit3("Color: ", (float*)&mat.Color);
	ImGui::Separator();

	ImGui::Text("Factors:");
	changed |= ImGui::SliderFloat("Color Intensity", (float*)&mat.ColorIntensity, 0.0f, 1.0f, "%.3f");
	changed |= ImGui::SliderFloat("AO Intensity", (float*)&mat.AoIntensity, 0.0f, 1.0f, "%.3f");
	changed |= ImGui::SliderFloat("Reflectivity", (float*)&mat.Reflectivity, 0.0f, 1.0f, "%.3f");
	changed |= ImGui::SliderFloat("Refraction Ration", (float*)&mat.Refraction_ration, 0.0f, 1.0f, "%.3f");

	ImGui::Text("Maps:");

	ChooseTexture(graphics, textures, "Base Map", mat.BaseMap);
	ChooseTexture(graphics, textures, "Ao Map", mat.AoMap);

	return changed;
}

bool ControlPhongMaterialImGui(PhongMaterial& mat, const char* name)
{
	bool changed = false;
	changed |= ImGui::ColorEdit3("Ambient Factors", (float*)&mat.Ambient);
	changed |= ImGui::ColorEdit3("Diffuse Factors", (float*)&mat.Diffuse);
	changed |= ImGui::ColorEdit3("Specular Factors", (float*)&mat.Specular);
	changed |= ImGui::ColorEdit3("Emissive Factors", (float*)&mat.Emissive);
	ImGui::Separator();
		
	changed |= ImGui::SliderFloat("Specular Exponent", (float*)&mat.SpecularChininess, 0.0f, 25.0f, "%.3f");

	return changed;
}
