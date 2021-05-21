#pragma once

#include <GraphicsCommon.hpp>
#include <Types.hpp>
#include <Utils.hpp>
#include <Assets.hpp>
#include <Platform.hpp>
#include <Graphics.hpp>
#include <GraphicsCommon.hpp>
#include <Memory.hpp>
#include <Utils.hpp>
#include <Assets.hpp>
#include <FileUtils.hpp>
#include <Platform.hpp>
#include <TextureCatalog.hpp>

#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <sstream>
#include <cstdint>
#include <fstream>
#include <stb_image.h>
#include <fmt/format.h>
#include <filesystem>

static LPSTR* CommandLineToArgvA(LPSTR lpCmdLine, INT* pNumArgs)
{
	int retval;
	retval = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, lpCmdLine, -1, NULL, 0);

	LPWSTR lpWideCharStr = (LPWSTR)malloc(retval * sizeof(WCHAR));
	retval = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, lpCmdLine, -1, lpWideCharStr, retval);
	
	if (!SUCCEEDED(retval))
	{
		free(lpWideCharStr);
		return NULL;
	}
	
	int numArgs;
	LPWSTR* args;
	args = CommandLineToArgvW(lpWideCharStr, &numArgs);
	free(lpWideCharStr);

	int storage = numArgs * sizeof(LPSTR);
	for (int i = 0; i < numArgs; ++i)
	{
		BOOL lpUsedDefaultChar = FALSE;
		retval = WideCharToMultiByte(CP_ACP, 0, args[i], -1, NULL, 0, NULL, &lpUsedDefaultChar);
		if (!SUCCEEDED(retval))
		{
			LocalFree(args);
			return NULL;
		}

		storage += retval;
	}

	LPSTR* result = (LPSTR*)LocalAlloc(LMEM_FIXED, storage);
	if (result == NULL)
	{
		LocalFree(args);
		return NULL;
	}

	int bufLen = storage - numArgs * sizeof(LPSTR);
	LPSTR buffer = ((LPSTR)result) + numArgs * sizeof(LPSTR);
	for (int i = 0; i < numArgs; ++i)
	{		
		BOOL lpUsedDefaultChar = FALSE;
		retval = WideCharToMultiByte(CP_ACP, 0, args[i], -1, buffer, bufLen, NULL, &lpUsedDefaultChar);
		if (!SUCCEEDED(retval))
		{
			LocalFree(result);
			LocalFree(args);
			return NULL;
		}

		result[i] = buffer;
		buffer += retval;
		bufLen -= retval;
	}

	LocalFree(args);

	*pNumArgs = numArgs;
	return result;

}

static std::vector<unsigned char> LoadFile(const std::string &t_filename)
{
    std::ifstream infile(t_filename.c_str(), std::ios::in | std::ios::ate | std::ios::binary);

	auto size = infile.tellg();
    infile.seekg(0, std::ios::beg);

	std::vector<unsigned char> v(static_cast<size_t>(size));
	infile.read((char*)&v[0], static_cast<std::streamsize>(size));

	return v;
 }

static std::string LoadFileIntoString(const std::string &t_filename)
{
	auto v = LoadFile(t_filename);
	return std::string(v.begin(), v.end());
}

static std::vector<std::string> SplitLine(const std::string &s, char delimiter)
{
    std::vector<std::string> tokens;

    size_t last = 0;
    size_t next = 0;

    while ((next = s.find(delimiter, last)) != std::string::npos)
    {
        tokens.push_back(s.substr(last, next - last));
        last = next + 1;
    }
    tokens.push_back(s.substr(last));
    return tokens;
}

static glm::i32vec3 GetIndexData(std::string part)
{
	auto parts = SplitLine(part, '/');
	return glm::i32vec3{
		std::stoi(parts[0].c_str()) - 1,
		std::stoi(parts[1].c_str()) - 1,
		std::stoi(parts[2].c_str()) - 1};
}

static bool StartsWith(const std::string &str, const std::string &prefix)
{
	return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

static std::string ReplaceAll(std::string str, const std::string &from, const std::string &to)
{
    if (from.empty()) return str;

    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();  // In case 'to' contains 'from', like
		// replacing 'x' with 'yx'
    }
    return str;
}

struct MaterialEditor
{
	struct CommandLineArguments
	{
		std::string Root{"resources"};
		std::string Input{"input.dxa"};
	};

};

struct Context
{
	HWND hWnd;
	MaterialEditor::CommandLineArguments Args;
	bool FullscreenMode;
	UINT WindowStyle;
	RECT WindowRect;
	Graphics Graphics;

	Renderer3D Renderer3D;
	
	TextureCatalog Textures;
	std::vector<MeshId> Meshes;
};

static void LoadObjMesh(Context& context, const char* path)
{
	auto content = LoadFileIntoString(path);
	std::stringstream stream(content);
	std::string line;

	std::vector<MtlVertex> VertexData;
	std::vector<uint32> IndexData;

	std::vector<glm::vec3> Pos;
	std::vector<glm::vec3> Norms;
	std::vector<glm::vec2> UVs;
	
	std::unordered_map<std::string, uint32> indexMap;
	
	VertexData.reserve(1024);
	IndexData.reserve(2048);
	Pos.reserve(1024);
	Norms.reserve(1024);
	UVs.reserve(1024);
	indexMap.reserve(2048);

	while(std::getline(stream, line, '\n'))
	{
		if (line[0] == '#') continue;
		
		if (line[0] == 'v' && line[1] == ' ')
		{
			auto parts = SplitLine(line, ' ');
			Pos.push_back(glm::vec3{
					std::stof(parts[1].c_str()),
					std::stof(parts[2].c_str()),
					std::stof(parts[3].c_str())});
		}
		else if (line[0] == 'v' && line[1] == 'n')
		{
			auto parts = SplitLine(line, ' ');
			Norms.push_back(glm::vec3{
					std::stof(parts[1].c_str()),
					std::stof(parts[2].c_str()),
					std::stof(parts[3].c_str())});
		}
		else if (line[0] == 'v' && line[1] == 't')
		{
			auto parts = SplitLine(line, ' ');
			UVs.push_back(glm::vec2{
					std::stof(parts[1].c_str()),
					std::stof(parts[2].c_str())});
		}
		else if (line[0] == 'f' && line[1] == ' ')
		{
			auto parts = SplitLine(line, ' ');

			const auto vtn1 = GetIndexData(parts[1]);
			const auto vtn2 = GetIndexData(parts[2]);
			const auto vtn3 = GetIndexData(parts[3]);
			
			MtlVertex nextVertex;
			
			if (indexMap.insert({parts[1], (uint32)VertexData.size()}).second)
			{
				nextVertex.pos = Pos[vtn1.x];
				nextVertex.normal = Norms[vtn1.z];
				nextVertex.uv = UVs[vtn1.y];
				VertexData.push_back(nextVertex);
			}
			
			if (indexMap.insert({parts[2], (uint32)VertexData.size()}).second)
			{
				nextVertex.pos = Pos[vtn2.x];
				nextVertex.normal = Norms[vtn2.z];
				nextVertex.uv = UVs[vtn2.y];
				VertexData.push_back(nextVertex);
			}
			
			if (indexMap.insert({parts[3], (uint32)VertexData.size()}).second)
			{
				nextVertex.pos = Pos[vtn3.x];
				nextVertex.normal = Norms[vtn3.z];
				nextVertex.uv = UVs[vtn3.y];
				VertexData.push_back(nextVertex);
			}

			IndexData.push_back(indexMap[parts[1]]);
			IndexData.push_back(indexMap[parts[2]]);
			IndexData.push_back(indexMap[parts[3]]);

			if (parts.size() > 4)
			{
				const auto vtn4 = GetIndexData(parts[4]);
				if (indexMap.insert({parts[4], (uint32)VertexData.size()}).second)
				{
					nextVertex.pos = Pos[vtn4.x];
					nextVertex.normal = Norms[vtn4.z];
					nextVertex.uv = UVs[vtn4.y];
					VertexData.push_back(nextVertex);
				}

				IndexData.push_back(indexMap[parts[4]]);
				IndexData.push_back(indexMap[parts[1]]);
				IndexData.push_back(indexMap[parts[3]]);

			}
			
		}
    }

	auto vbo = NextVertexBufferId();
	auto ibo = NextIndexBufferId();
	MeshId nextMesh = (uint16)context.Meshes.size();

	context.Graphics.CreateVertexBuffer(vbo, sizeof(MtlVertex), VertexData.data(), (uint32)(sizeof(MtlVertex) * VertexData.size()), false);
	context.Graphics.CreateIndexBuffer(ibo, IndexData.data(), (uint32)IndexData.size(), false);

	GPUGeometry meshGeometry;
	meshGeometry.Ibo = ibo;
	meshGeometry.Vbo= vbo;
	meshGeometry.Description.VertexCount = (uint32)VertexData.size();
	meshGeometry.Description.IndexCount= (uint32)IndexData.size();

	Mesh mesh;
	mesh.Geometry = meshGeometry;
	mesh.Material= 1;

	context.Renderer3D.MeshData.Meshes.insert({nextMesh, mesh});
	context.Meshes.push_back(nextMesh);
}

static uint32 CUBE;
static uint32 PLANE;
static uint32 LINES;
static uint32 CYLINDER;
static uint32 SPHERE;
static uint32 AXIS;
static uint32 POINTLIGHT;
static uint32 SPOTLIGHT;

static void Init(Context& context)
{
	auto Graphics = &context.Graphics;
	context.Renderer3D.InitRenderer(Graphics);

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


	const static float pov = 65.0f;
	const static float nearPlane = 0.0001f;
	const static float farPlane = 1000.0f;
	
	context.Renderer3D.SetupProjection(glm::perspective(pov, 1080.0f / 720.0f, nearPlane, farPlane));

	context.Textures.LoadedTextures.reserve(64);
	context.Textures.LoadedCubes.reserve(16);
}

static void Update(Context& context, float dt)
{
	auto Graphics = &context.Graphics;

	Graphics->SetViewport(0, 0, 1080, 720);
	Graphics->SetBlendingState(BS_AlphaBlending);
	Graphics->ResetRenderTarget();
	Graphics->ClearBuffer(0.0f, 0.0f, 0.0f);
	Graphics->ClearZBuffer();
	Graphics->SetDepthStencilState(DSS_Normal);
	Graphics->SetRasterizationState(RS_NORMAL);

	ControlCameraFPS(context.Renderer3D.CurrentCamera, dt);

	context.Renderer3D.UpdateCamera();

	context.Renderer3D.BeginScene(SC_DEBUG_COLOR);
	context.Renderer3D.DrawDebugGeometry(AXIS, { 0.0f, 0.0f, 0.0f }, glm::vec3(1.0f));

	context.Renderer3D.DrawSkyBox(context.Textures.LoadedTextures.back().Handle);
}


// camera
