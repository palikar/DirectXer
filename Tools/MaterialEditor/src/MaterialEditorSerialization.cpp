
#include "MaterialEditor.hpp"
#include "Utils.hpp"

static size_t PAHTS_MEMORY = Kilobytes(4);

static TextureId FindTexId(Context& context, fs::path path)
{
	return std::find_if(context.Textures.LoadedTextures.begin(), context.Textures.LoadedTextures.end(),
						[path](auto l) { return fs::equivalent(l.Path, path); })->Handle;

}

MtlMaterial LoadMtlMaterial(Context& context, fs::path path)
{
	auto content = LoadFileIntoString(path.string());
	std::stringstream stream(content);
	std::string line;

	std::string newMatName;
	std::string texName;

	MtlMaterial newMat{ 0 };

	const ShaderConfiguration configs[] = {
		SC_MTL_1,
		SC_MTL_2
	};

	while (std::getline(stream, line, '\n'))
	{
		if (line[0] == '#') continue;

		if (StartsWith(line, "newmtl"))
		{
			auto parts = SplitLine(line, ' ');
			newMatName = ReplaceAll(parts[1], ".", "_");
			newMat = { 0 };
		}
		else if (StartsWith(line, "Ns"))
		{
			auto parts = SplitLine(line, ' ');

			newMat.Ns = std::stof(parts[1].c_str());
			newMat.illum |= NS_FACT_MASK;
		}
		else if (StartsWith(line, "Ni"))
		{
			auto parts = SplitLine(line, ' ');

			newMat.Ni = std::stof(parts[1].c_str());
		}
		else if (StartsWith(line, "d"))
		{
			auto parts = SplitLine(line, ' ');

			newMat.d = std::stof(parts[1].c_str());
			newMat.illum |= D_FACT_MASK;
		}
		else if (StartsWith(line, "illum"))
		{
			auto parts = SplitLine(line, ' ');

			newMat.illum |= (uint32)std::stoi(parts[1].c_str());
			assert((newMat.illum & 0xFF) <= 2);
			newMat.Program = configs[(newMat.illum & 0xFF) - 1];
		}
		else if (StartsWith(line, "Ka"))
		{
			auto parts = SplitLine(line, ' ');

			newMat.Ka = {
				std::stof(parts[1].c_str()),
				std::stof(parts[2].c_str()),
				std::stof(parts[3].c_str())
			};
			newMat.illum |= KA_FACT_MASK;
		}
		else if (StartsWith(line, "Kd"))
		{
			auto parts = SplitLine(line, ' ');

			newMat.Kd = {
				std::stof(parts[1].c_str()),
				std::stof(parts[2].c_str()),
				std::stof(parts[3].c_str())
			};

			newMat.illum |= KD_FACT_MASK;
		}
		else if (StartsWith(line, "Ks"))
		{
			auto parts = SplitLine(line, ' ');

			newMat.Ks = {
				std::stof(parts[1].c_str()),
				std::stof(parts[2].c_str()),
				std::stof(parts[3].c_str())
			};
			newMat.illum |= KS_FACT_MASK;
		}
		else if (StartsWith(line, "Ke"))
		{
			auto parts = SplitLine(line, ' ');

			newMat.Ke = {
				std::stof(parts[1].c_str()),
				std::stof(parts[2].c_str()),
				std::stof(parts[3].c_str())
			};
		}
		else if (StartsWith(line, "map_Kd"))
		{
			
			newMat.KdMap = FindTexId(context, ReplaceAll(line,"map_Kd ", ""));
			newMat.illum |= KD_TEX_MASK;
			newMat.illum &= ~KD_FACT_MASK;
		}
	}

	return newMat;

}

PhongMaterial LoadPhongMaterial(Context& context, fs::path path)
{
	auto content = LoadFileIntoString(path.string());
	std::stringstream stream(content);
	std::string line;

	PhongMaterial newMat;
	memset(&newMat, 0, sizeof(PhongMaterial));

	while (std::getline(stream, line, '\n'))
	{
		if (line[0] == '#') continue;

		if (StartsWith(line, "Ambient "))
		{
			auto parts = SplitLine(line, ' ');
			
			newMat.Ambient = {
				std::stof(parts[1].c_str()),
				std::stof(parts[2].c_str()),
				std::stof(parts[3].c_str())
			};
		}
		else if (StartsWith(line, "Diffuse "))
		{
			auto parts = SplitLine(line, ' ');

			newMat.Diffuse = {
				std::stof(parts[1].c_str()),
				std::stof(parts[2].c_str()),
				std::stof(parts[3].c_str())
			};
		}
		else if (StartsWith(line, "Specular "))
		{
			auto parts = SplitLine(line, ' ');

			newMat.Specular = {
				std::stof(parts[1].c_str()),
				std::stof(parts[2].c_str()),
				std::stof(parts[3].c_str())
			};
		}
		else if (StartsWith(line, "Emissive "))
		{
			auto parts = SplitLine(line, ' ');

			newMat.Emissive = {
				std::stof(parts[1].c_str()),
				std::stof(parts[2].c_str()),
				std::stof(parts[3].c_str())
			};
		}
		else if (StartsWith(line, "SpecularExponent "))
		{
			auto parts = SplitLine(line, ' ');

			newMat.SpecularChininess = std::stof(parts[1].c_str());
		}

	}

	return newMat;
}

TexturedMaterial LoadTexturedMaterial(Context& context, fs::path path)
{
	auto content = LoadFileIntoString(path.string());
	std::stringstream stream(content);
	std::string line;

	TexturedMaterial newMat;
	memset(&newMat, 0, sizeof(TexturedMaterial));

	while (std::getline(stream, line, '\n'))
	{
		if (line[0] == '#') continue;

		if (StartsWith(line, "Color "))
		{
			auto parts = SplitLine(line, ' ');
			
			newMat.Color = {
				std::stof(parts[1].c_str()),
				std::stof(parts[2].c_str()),
				std::stof(parts[3].c_str()),
				1.0f
			};
		}
		else if (StartsWith(line, "ColorIntensity "))
		{
			auto parts = SplitLine(line, ' ');

			newMat.ColorIntensity = std::stof(parts[1].c_str());
		}
		else if (StartsWith(line, "AoIntensity "))
		{
			auto parts = SplitLine(line, ' ');

			newMat.AoIntensity = std::stof(parts[1].c_str());
		}
		else if (StartsWith(line, "Reflectivity "))
		{
			auto parts = SplitLine(line, ' ');
			
			newMat.Reflectivity = std::stof(parts[1].c_str());
		}
		else if (StartsWith(line, "Refraction_ration "))
		{
			auto parts = SplitLine(line, ' ');
			
			newMat.Refraction_ration = std::stof(parts[1].c_str());
		}
		else if (StartsWith(line, "AoMap "))
		{
			newMat.AoMap = FindTexId(context, ReplaceAll(ReplaceAll(line, "AoMap ", ""), "\r", ""));
		}
		else if (StartsWith(line, "BaseMap "))
		{
			newMat.BaseMap = FindTexId(context, ReplaceAll(ReplaceAll(line, "BaseMap ", ""), "\r", ""));
		}
	}

	return newMat;
}

void LoadMaterial(Context& context, fs::path path)
{
	TempFormater formater;
	MaterialEditEntry basicEntry;
	DXLOG("[RES] Loading {}", path.string().c_str());
	
	if (path.extension() == ".mtl")
	{
		auto basicMat = LoadMtlMaterial(context, path);
		InitMaterial(&context.Graphics, basicMat, path.stem().string().c_str());
		context.Renderer3D.MeshData.Materials.MtlMaterials.push_back(basicMat);

		basicEntry.Type = MT_MTL;
		basicEntry.Id = basicMat.Id;
		basicEntry.Mtl = &context.Renderer3D.MeshData.Materials.MtlMaterials.back();

	}
	else if (path.extension() == ".mtl_phong")
	{
		auto basicMat = LoadPhongMaterial(context, path);
		basicMat.Id = ++NextMaterialId;
		InitMaterial(&context.Graphics, basicMat, path.stem().string().c_str());
		context.Renderer3D.MeshData.Materials.PhongMaterials.push_back(basicMat);

		basicEntry.Type = MT_PHONG;
		basicEntry.Id = basicMat.Id;
		basicEntry.Phong = &context.Renderer3D.MeshData.Materials.PhongMaterials.back();
	}
	else if (path.extension() == ".mtl_tex")
	{
		auto basicMat = LoadTexturedMaterial(context, path);
		basicMat.EnvMap = context.Textures.LoadedCubes[context.CurrentMapIndex].Handle;
		basicMat.Id = ++NextMaterialId;
		InitMaterial(&context.Graphics, basicMat, path.stem().string().c_str());
		context.Renderer3D.MeshData.Materials.TexMaterials.push_back(basicMat);

		basicEntry.Type = MT_TEXTURED;
		basicEntry.Id = basicMat.Id;
		basicEntry.Tex = &context.Renderer3D.MeshData.Materials.TexMaterials.back();
	}
	
	basicEntry.Name = path.stem().string().c_str();
	context.Materials.push_back(basicEntry);
}

void LoadSession(Context& context, const char* path)
{
	DXLOG("[RES] Loading {}", path);
	
	TempFormater formater;
	SerializationContext serial = { 0 };
	SessionSaveContext saveContext;
	saveContext.MaterialsCount = (uint32)context.Materials.size();
	auto tempArena = Memory::GetTempArena(PAHTS_MEMORY);
	saveContext.Paths = tempArena.Memory;
	Defer {
		Memory::DestoryTempArena(tempArena);
	};
	
	serial.Lighting = &context.Renderer3D.LightingSetup.LightingData;
	serial.Camera = &context.Renderer3D.CurrentCamera;

	serial.Datas[0].Size = PAHTS_MEMORY;
	serial.Datas[0].Data = saveContext.Paths;

	serial.Datas[1].Size = sizeof(32);
	serial.Datas[1].Data = &saveContext.MaterialsCount;

	serial.Datas[2].Size = sizeof(CameraControlState);
	serial.Datas[2].Data = &context.CameraState;

	Serialization::LoadFromFile(path, serial);

	for (uint32 i = 0; i < saveContext.MaterialsCount; ++i)
	{
		LoadMaterial(context, saveContext.Paths + i*64);
	}

	context.Renderer3D.MeshData.Materials.GenerateProxies();
}

static std::string TexPath(Context& context, TextureId tex)
{
	auto path = std::find_if(context.Textures.LoadedTextures.begin(), context.Textures.LoadedTextures.end(),
							 [tex](auto t) { return t.Handle == tex;})->Path.data();

	return fs::absolute(path).string();
}
	
void SaveMaterial(Context& context, MtlMaterial mat, const char* path, const char* name)
{
	TempFormater formater;
	std::ofstream output;
	output.open(path);

	output << formater.Format("newmtl {}\n", name);
	output << formater.Format("Ka {} {} {}\n", mat.Ka.x, mat.Ka.y, mat.Ka.z);
	output << formater.Format("Kd {} {} {}\n", mat.Kd.x, mat.Kd.y, mat.Kd.z);
	output << formater.Format("Ke {} {} {}\n", mat.Ke.x, mat.Ke.y, mat.Ke.z);
	output << formater.Format("Ks {} {} {}\n", mat.Ks.x, mat.Ks.y, mat.Ks.z);
	output << formater.Format("Ns {}\n", mat.Ns);
	output << formater.Format("d {}\n", mat.d);
	output << formater.Format("Ni {}\n", mat.Ni);
	output << formater.Format("illum {}\n", mat.illum & 255);

	if ((mat.illum & KA_TEX_MASK) > 0)
		output << formater.Format("map_Ka {}\n", TexPath(context, mat.KaMap));

	if ((mat.illum & KD_TEX_MASK) > 0)
		output << formater.Format("map_Kd {}\n", TexPath(context, mat.KdMap));

	if ((mat.illum & KS_TEX_MASK) > 0)
		output << formater.Format("map_Ks {}\n", TexPath(context, mat.KsMap));

	if ((mat.illum & NS_TEX_MASK) > 0)
		output << formater.Format("map_Ns {}\n", TexPath(context, mat.NsMap));

	if ((mat.illum & D_TEX_MASK) > 0)
		output << formater.Format("map_d {}\n", TexPath(context, mat.dMap));
	

	output.close();
}

void SaveMaterial(Context& context, PhongMaterial mat, const char* path, const char* name)
{
	TempFormater formater;
	std::ofstream output;
	output.open(path);

	output << formater.Format("newmtl {}\n", name);
	output << formater.Format("Ambient {} {} {}\n", mat.Ambient.x, mat.Ambient.y, mat.Ambient.z);
	output << formater.Format("Diffuse {} {} {}\n", mat.Diffuse.x, mat.Diffuse.y, mat.Diffuse.z);
	output << formater.Format("Specular {} {} {}\n", mat.Specular.x, mat.Specular.y, mat.Specular.z);
	output << formater.Format("Emissive {} {} {}\n", mat.Emissive.x, mat.Emissive.y, mat.Emissive.z);
	output << formater.Format("SpecularExponent {}\n", mat.SpecularChininess);	

	output.close();
}

void SaveMaterial(Context& context, TexturedMaterial mat, const char* path, const char* name)
{
	TempFormater formater;
	std::ofstream output;
	output.open(path);

	output << formater.Format("newmtl {}\n", name);
	output << formater.Format("Color {} {} {}\n", mat.Color.x, mat.Color.y, mat.Color.z);
	output << formater.Format("ColorIntensity {}\n", mat.ColorIntensity);	
	output << formater.Format("AoIntensity {}\n", mat.AoIntensity);	
	output << formater.Format("Reflectivity {}\n", mat.Reflectivity);	
	output << formater.Format("Refraction_ration {}\n", mat.Refraction_ration);

	if (mat.BaseMap != 0) output << formater.Format("BaseMap {}\n", TexPath(context, mat.BaseMap));
	if (mat.AoMap != 0) output << formater.Format("AoMap {}\n", TexPath(context, mat.AoMap));
	
	output.close();
}

void SaveSession(Context& context, const char* path)
{
	TempFormater formater;
	SerializationContext serial = { 0 };
	SessionSaveContext saveContext;

	saveContext.MaterialsCount = (uint32)context.Materials.size();
	auto tempArena = Memory::GetTempArena(PAHTS_MEMORY);
	saveContext.Paths = tempArena.Memory;
	Defer {
		Memory::DestoryTempArena(tempArena);
	};
	
	for (size_t i = 0; i < context.Materials.size(); ++i)
	{
		auto mat = context.Materials[i];
		
		std::string path;

		if (mat.Type == MT_MTL)
		{
			path = formater.Format("resources/materials/{}.mtl", mat.Name);
			SaveMaterial(context, *mat.Mtl, path.c_str(), mat.Name.c_str());
		}
		else if (mat.Type == MT_TEXTURED)
		{
			path = formater.Format("resources/materials/{}.mtl_tex", mat.Name);
			SaveMaterial(context, *mat.Tex, path.c_str(), mat.Name.c_str());
		}
		else if (mat.Type == MT_PHONG)
		{
			path = formater.Format("resources/materials/{}.mtl_phong", mat.Name);
			SaveMaterial(context, *mat.Phong, path.c_str(), mat.Name.c_str());
		}

		strcpy(saveContext.Paths + i*64, path.c_str());
	}

	serial.Lighting = &context.Renderer3D.LightingSetup.LightingData;
	serial.Camera = &context.Renderer3D.CurrentCamera;

	serial.Datas[0].Size = PAHTS_MEMORY;
	serial.Datas[0].Data = saveContext.Paths;

	serial.Datas[1].Size = sizeof(32);
	serial.Datas[1].Data = &saveContext.MaterialsCount;

	serial.Datas[2].Size = sizeof(CameraControlState);
	serial.Datas[2].Data = &context.CameraState;

	Serialization::DumpToFile(path, serial);
}
