
#include "Utils.hpp"
#include "MaterialEditor.hpp"

namespace fs = std::filesystem;

static uint32 CUBE;
static uint32 PLANE;
static uint32 LINES;
static uint32 CYLINDER;
static uint32 SPHERE;
static uint32 AXIS;
static uint32 POINTLIGHT;
static uint32 SPOTLIGHT;

const static float pov = 65.0f;
const static float nearPlane = 0.0001f;
const static float farPlane = 10000.0f;

static void LoadSession(Context& context, const char* path);
	
void Init(Context& context)
{
	auto Graphics = &context.Graphics;
	context.Renderer3D.InitRenderer(Graphics);
	context.Textures.LoadedTextures.reserve(64);
	context.Textures.LoadedCubes.reserve(16);

	const char* Envs[] = {
		"resources/night_sky",
		"resources/sky",
	};

	const char* Texs[] = {
		"resources/floor_color.png",
		"resources/rocks_color.png",
		"resources/rocks_normal.png",
		"resources/rocks_ao.png",
		"resources/checker.png",
		"resources/bricks_color.png",
		"resources/bricks_ao.png",
		"resources/bricks_normal.png",
		"resources/dummyTexture.png",
	};

	const char* Meshes[] = {
		"resources/models/first_tree.obj",
		"resources/models/simple_monkey.obj",
		"resources/models/simple_sphere.obj",
	};

	for (size_t i = 0; i < Size(Meshes); i++)
	{
		LoadObjMesh(context, Meshes[i]);
	}

	context.Textures.LoadTextures(Graphics, Texs, (uint32)Size(Texs));
	context.Textures.LoadCubes(Graphics, Envs, (uint32)Size(Envs));

	Memory::EstablishTempScope(Megabytes(4));
	{
		DebugGeometryBuilder builder;
		builder.Init(8);
		CUBE = builder.InitCube(CubeGeometry{}, glm::vec3{ 1.0f, 0.0f, 0.0f });
		PLANE = builder.InitPlane(PlaneGeometry{}, glm::vec3{ 0.0f, 1.0f, 0.0f });
		SPHERE = builder.InitSphere(SphereGeometry{}, glm::vec3{ 0.0f, 1.0f, 0.0f });
		CYLINDER = builder.InitCylinder(CylinderGeometry{ 0.25, 0.25, 1.5 }, glm::vec3{ 1.0f, 1.0f, 0.0f });
		LINES = builder.InitLines(LinesGeometry{}, glm::vec3{ 0.8f, 0.8f, 0.8f });
		AXIS = builder.InitAxisHelper();
		POINTLIGHT = builder.InitPointLightHelper();
		SPOTLIGHT = builder.InitSpotLightHelper();

		context.Renderer3D.InitDebugGeometry(builder);
	}
	Memory::EndTempScope();

	context.Renderer3D.CurrentCamera.Pos = { 1.0f, 0.5f, 1.0f };
	context.Renderer3D.CurrentCamera.lookAt({ 0.0f, 0.0f, 0.0f });
	context.Renderer3D.SetupProjection(glm::perspective(pov, 1080.0f / 720.0f, nearPlane, farPlane));

	context.Renderer3D.MeshData.Materials.Init();
	context.Renderer3D.InitLighting();
	
	context.Materials.reserve(32);

	MtlMaterial basicMat = { 0 };
	basicMat.illum = 2;
	basicMat.Ka = {0.2f, 0.2f, 0.2f};
	basicMat.Kd = {0.7f, 0.0f, 0.2f};
	basicMat.Ks = {0.8f, 0.8f, 0.8f};
	basicMat.Id = 1;
	InitMaterial(Graphics, basicMat, "Default Material");

	context.Renderer3D.MeshData.Materials.MtlMaterials.push_back(basicMat);	
	context.Renderer3D.MeshData.Materials.GenerateProxies();

	LoadSession(context, "basic.dxms");

	if(context.Materials.empty())
	{
		MaterialEditEntry basicEntry;
		basicEntry.Name = "Default_MTL";
		basicEntry.Type = MT_MTL;
		basicEntry.Id = basicMat.Id;
		basicEntry.Mtl = &context.Renderer3D.MeshData.Materials.MtlMaterials[0];
		context.Materials.push_back(basicEntry);
	}

	
}

inline static MaterialId NextMaterialId = 1;

static void NewMtlMaterial(Context& context)
{
	const size_t mtlCount = context.Renderer3D.MeshData.Materials.MtlMaterials.size(); 
	
	MtlMaterial basicMat = { 0 };
	memset(&basicMat, 0, sizeof(MtlMaterial));
	basicMat.illum = 2;
	basicMat.Ka = {0.2f, 0.2f, 0.2f};
	basicMat.Kd = {0.7f, 0.0f, 0.2f};
	basicMat.Ks = {0.8f, 0.8f, 0.8f};
	basicMat.Id = ++NextMaterialId;

	TempFormater formater;
	InitMaterial(&context.Graphics, basicMat, formater.Format("Mtl_Material_{}", mtlCount));

	context.Renderer3D.MeshData.Materials.MtlMaterials.push_back(basicMat);	
	context.Renderer3D.MeshData.Materials.GenerateProxy(context.Renderer3D.MeshData.Materials.MtlMaterials.back());

	MaterialEditEntry basicEntry;
	basicEntry.Type = MT_MTL;
	basicEntry.Id = basicMat.Id;
	basicEntry.Mtl = &context.Renderer3D.MeshData.Materials.MtlMaterials.back();
	basicEntry.Name = formater.Format("Mtl_Material_{}", mtlCount);

	context.Materials.push_back(basicEntry);

	context.CurrentMaterialIndex = (int)context.Materials.size() - 1;
}

static void NewPhongMaterial(Context& context)
{
	const size_t phongCount = context.Renderer3D.MeshData.Materials.PhongMaterials.size(); 
	
	PhongMaterial basicMat;
	memset(&basicMat, 0, sizeof(PhongMaterial));
	basicMat.Ambient = {0.2f, 0.2f, 0.2f};
	basicMat.Diffuse = {0.7f, 0.0f, 0.2f};
	basicMat.Specular = {0.8f, 0.8f, 0.8f};
	basicMat.Emissive = {0.8f, 0.8f, 0.8f};
	basicMat.SpecularChininess = 1.3f;
	
	basicMat.Id = ++NextMaterialId;

	TempFormater formater;
	InitMaterial(&context.Graphics, basicMat, formater.Format("Phong_Material_{}", phongCount));

	context.Renderer3D.MeshData.Materials.PhongMaterials.push_back(basicMat);	
	context.Renderer3D.MeshData.Materials.GenerateProxy(context.Renderer3D.MeshData.Materials.PhongMaterials.back());

	MaterialEditEntry basicEntry;
	basicEntry.Type = MT_PHONG;
	basicEntry.Id = basicMat.Id;
	basicEntry.Phong = &context.Renderer3D.MeshData.Materials.PhongMaterials.back();
	basicEntry.Name = formater.Format("Phong_Material_{}", phongCount);

	context.Materials.push_back(basicEntry);
	context.CurrentMaterialIndex = (int)context.Materials.size() - 1;
}

static void NewTexMaterial(Context& context)
{
	const size_t texCount = context.Renderer3D.MeshData.Materials.TexMaterials.size(); 
	
	TexturedMaterial basicMat;
	memset(&basicMat, 0, sizeof(TexturedMaterial));
	basicMat.Color = {0.7f, 0.1f, 0.1f, 0.5f};
	basicMat.ColorIntensity =  0.0f;
	basicMat.AoIntensity =  0.0f;
	basicMat.Reflectivity =  0.0f;
	basicMat.Refraction_ration = 0.0;

	basicMat.BaseMap = context.Textures.LoadedTextures[0].Handle;
	basicMat.AoMap = context.Textures.LoadedTextures[0].Handle;
	basicMat.EnvMap = context.Textures.LoadedTextures[context.CurrentMapIndex].Handle;
	
	basicMat.Id = ++NextMaterialId;

	TempFormater formater;
	InitMaterial(&context.Graphics, basicMat, formater.Format("Textured_Material_{}", texCount));

	context.Renderer3D.MeshData.Materials.TexMaterials.push_back(basicMat);	
	context.Renderer3D.MeshData.Materials.GenerateProxy(context.Renderer3D.MeshData.Materials.TexMaterials.back());

	MaterialEditEntry basicEntry;
	basicEntry.Type = MT_TEXTURED;
	basicEntry.Id = basicMat.Id;
	basicEntry.Tex = &context.Renderer3D.MeshData.Materials.TexMaterials.back();
	basicEntry.Name = formater.Format("Textured_Material_{}", texCount);

	context.Materials.push_back(basicEntry);
	context.CurrentMaterialIndex = (int)context.Materials.size() - 1;
}

static void ControlCurrentMaterial(Context& context)
{

	MaterialEditEntry mat = context.Materials[context.CurrentMaterialIndex];
	bool changed = false;
	if(mat.Type == MT_MTL)
	{
		changed = ControlMtlMaterialImGui(*mat.Mtl, mat.Name.c_str(), context.Textures, &context.Graphics);
		
	}
	else if(mat.Type == MT_PHONG)
	{
		changed = ControlPhongMaterialImGui(*mat.Phong, mat.Name.c_str());
	}
	else if(mat.Type == MT_TEXTURED)
	{
		changed = ControlTexturedMaterialImGui(*mat.Tex, mat.Name.c_str(), context.Textures, &context.Graphics);
	}

	if (changed) context.Renderer3D.MeshData.Materials.Update(&context.Graphics, mat.Id);
}

static void RemoveCurrentMat(Context& context)
{
	context.Materials.erase(context.Materials.begin() + context.CurrentMaterialIndex);
	if (context.CurrentMaterialIndex >= context.Materials.size()) context.CurrentMaterialIndex -= 1;
}

static std::string TexPath(Context& context, TextureId tex)
{
	auto path = std::find_if(context.Textures.LoadedTextures.begin(), context.Textures.LoadedTextures.end(),
				 [tex](auto t) { return t.Handle == tex;})->Path.data();

	return fs::absolute(path).string();
}
	
static void SaveMaterial(Context& context, MtlMaterial mat, const char* path, const char* name)
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

static void SaveMaterial(Context& context, PhongMaterial mat, const char* path, const char* name)
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

static void SaveMaterial(Context& context, TexturedMaterial mat, const char* path, const char* name)
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

static void SaveSession(Context& context)
{
	TempFormater formater;
	SerializationContext serial = { 0 };
	SessionSaveContext saveContext;

	saveContext.MaterialsCount = (uint32)context.Materials.size();
	size_t PAHTS_MEMORY = Kilobytes(4);
	saveContext.Paths = (char*)malloc(PAHTS_MEMORY);
	
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

	Serialization::DumpToFile("basic.dxms", serial);
	
	free(saveContext.Paths);
}

static TextureId FindTexId(Context& context, fs::path path)
{
	return std::find_if(context.Textures.LoadedTextures.begin(), context.Textures.LoadedTextures.end(),
						[path](auto l) { return fs::equivalent(l.Path, path); })->Handle;

}

static MtlMaterial LoadMtlMaterial(Context& context, fs::path path)
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

static PhongMaterial LoadPhongMaterial(Context& context, fs::path path)
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

static TexturedMaterial LoadTexturedMaterial(Context& context, fs::path path)
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

static void LoadMaterial(Context& context, fs::path path)
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

static void LoadSession(Context& context, const char* path)
{
	DXLOG("[RES] Loading {}", path);
	
	TempFormater formater;
	SerializationContext serial = { 0 };
	SessionSaveContext saveContext;

	saveContext.MaterialsCount = (uint32)context.Materials.size();
	size_t PAHTS_MEMORY = Kilobytes(4);
	saveContext.Paths = (char*)malloc(PAHTS_MEMORY);
	
	serial.Lighting = &context.Renderer3D.LightingSetup.LightingData;
	serial.Camera = &context.Renderer3D.CurrentCamera;

	serial.Datas[0].Size = PAHTS_MEMORY;
	serial.Datas[0].Data = saveContext.Paths;

	serial.Datas[1].Size = sizeof(32);
	serial.Datas[1].Data = &saveContext.MaterialsCount;

	Serialization::LoadFromFile(path, serial);


	for (uint32 i = 0; i < saveContext.MaterialsCount; ++i)
	{
		LoadMaterial(context, saveContext.Paths + i*64);
	}

	context.Renderer3D.MeshData.Materials.GenerateProxies();

	free(saveContext.Paths);
}

void Update(Context& context, float dt)
{
	auto* currentMesh = &context.Meshes[context.CurrentMeshIndex];
	auto& Renderer3D = context.Renderer3D;
	
	if (ImGui::CollapsingHeader("Mesh Config"))
	{
		ImGui::Combo("Mesh", &context.CurrentMeshIndex, context.MeshNames.data(), (int)context.MeshNames.size());
		currentMesh = &context.Meshes[context.CurrentMeshIndex];
		
		ImGui::SliderFloat("Scale", (float*)&currentMesh->Scale, 0.0f, 5.0f, "%.2f");
		ImGui::SliderFloat3("Position", (float*)&currentMesh->Position, -5.0f, 5.0f, "%.2f");
	}

	if (ImGui::CollapsingHeader("Environment Map"))
	{
		if (ImGui::BeginCombo("Map", context.Textures.LoadedCubes[context.CurrentMapIndex].Name.data()))
		{
			for (int i = 0; i < (int)context.Textures.LoadedCubes.size(); ++i)
			{
				auto& tex = context.Textures.LoadedCubes[i];
				if (ImGui::Selectable(tex.Name.data(), i == context.CurrentMapIndex))
				{
					context.CurrentMapIndex = i;
				}
			}
			ImGui::EndCombo();
		}
	}

	ControlLightingImGui(context.Renderer3D.LightingSetup.LightingData);

	if (ImGui::CollapsingHeader("Ligting Movement"))
	{

		ImGui::Checkbox("Movement", &context.LightHelpersState.Updating);
		ImGui::Separator();
		
		ImGui::Text("Pointlight 1:");
		ImGui::SliderFloat("Radius[1]", &context.LightHelpersState.PointLight1Radius, 0.0f, 10.0f, "%.2f");
		ImGui::SliderFloat("Height[1]", &context.LightHelpersState.PointLight1Height, -5.0f, 10.0f, "%.2f");

		ImGui::Separator();

		ImGui::Text("Pointlight 2:");
		ImGui::SliderFloat("Radius[2]", &context.LightHelpersState.PointLight2Radius, 0.0f, 10.0f, "%.2f");
		ImGui::SliderFloat("Height[2]", &context.LightHelpersState.PointLight2Height, -5.0f, 10.0f, "%.2f");

		ImGui::Separator();

		ImGui::Text("Spotlight:");
		ImGui::SliderFloat("Height[3]", &context.LightHelpersState.SpotHeight, 0.0f, 5.0f, "%.2f");
	}

	if (ImGui::Button("Save session")) SaveSession(context);

	ImGui::Begin("Materials");

	ImGui::Text("Create New Material:");
	ImGui::SameLine();
	if (ImGui::Button("MTL")) NewMtlMaterial(context);
	ImGui::SameLine();
	if (ImGui::Button("Phong")) NewPhongMaterial(context);
	ImGui::SameLine();
	if (ImGui::Button("Textured")) NewTexMaterial(context);
	
	ImGui::Separator();
	ImGui::Text("Material Configuration");
	if (ImGui::BeginCombo("Material", context.Materials[context.CurrentMaterialIndex].Name.c_str()))
	{
		for (size_t i = 0; i < context.Materials.size(); ++i)
		{
			auto& mat = context.Materials[i];
			if (ImGui::Selectable(mat.Name.c_str(), i == context.CurrentMaterialIndex))
			{
				context.CurrentMaterialIndex = (int)i;
			}
		}

		ImGui::EndCombo();
	}
	ImGui::SameLine();
	if (ImGui::Button("Remove")) RemoveCurrentMat(context);
	

	ControlCurrentMaterial(context);
	ImGui::End();


	if (context.LightHelpersState.Updating) UpdateTime(dt, context.T);
	
	const float lightX1 = std::sin(context.T) * context.LightHelpersState.PointLight1Radius;
	const float lightY1 = std::cos(context.T) * context.LightHelpersState.PointLight1Radius;
	const float lightX2 = std::sin(context.T + PI) * context.LightHelpersState.PointLight2Radius;
	const float lightY2 = std::cos(context.T + PI) * context.LightHelpersState.PointLight2Radius;

	glm::vec3 light1Pos{lightX1, context.LightHelpersState.PointLight1Height, lightY1};
	glm::vec3 light2Pos{lightX2, context.LightHelpersState.PointLight2Height, lightY2};
	glm::vec3 spotLight{ 0.0f, context.LightHelpersState.SpotHeight, 0.0f };
	
	Renderer3D.LightingSetup.LightingData.pointLights[0].Position = glm::vec4(light1Pos, 0.0f);
	Renderer3D.LightingSetup.LightingData.pointLights[1].Position = glm::vec4(light2Pos, 0.0f);

	ControlCameraOrbital(context.Renderer3D.CurrentCamera, dt);
	context.Renderer3D.UpdateCamera();
	
	Renderer3D.UpdateLighting();

	auto Graphics = &context.Graphics;

	Graphics->SetBlendingState(BS_AlphaBlending);
	Graphics->ResetRenderTarget();
	Graphics->ClearBuffer(0.0f, 0.0f, 0.0f);
	Graphics->ClearZBuffer();
	Graphics->SetDepthStencilState(DSS_Normal);
	Graphics->SetRasterizationState(RS_NORMAL);

	context.Renderer3D.BeginScene(SC_DEBUG_COLOR);
	context.Renderer3D.DrawDebugGeometry(AXIS, { 0.0f, 0.0f, 0.0f }, glm::vec3(1.0f));
	Graphics->SetRasterizationState(RS_DEBUG);

	if (Renderer3D.LightingSetup.LightingData.pointLights[0].Active)
		Renderer3D.DrawDebugGeometry(POINTLIGHT, light1Pos, glm::vec3(1.0f));
	
	if (Renderer3D.LightingSetup.LightingData.pointLights[1].Active)
		Renderer3D.DrawDebugGeometry(POINTLIGHT, light2Pos, glm::vec3(1.0f));
	
	Graphics->SetRasterizationState(RS_NORMAL);

	if (Renderer3D.LightingSetup.LightingData.spotLights[0].Active)
		Renderer3D.DrawDebugGeometry(SPOTLIGHT, spotLight, glm::vec3(5.0f));


	Renderer3D.EnableLighting();

	context.Renderer3D.MeshData.Materials.Bind(&context.Graphics, context.Materials[context.CurrentMaterialIndex].Id);
	
	context.Renderer3D.DrawMesh(currentMesh->Id, currentMesh->Position, glm::vec3(currentMesh->Scale));
	context.Renderer3D.DrawSkyBox(context.Textures.LoadedCubes[context.CurrentMapIndex].Handle);
}
