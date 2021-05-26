

#include "Utils.hpp"
#include "MaterialEditor.hpp"

void LoadObjMesh(Context& context, const char* path)
{
	DXLOG("[RES] Loading {}", path);
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
	context.Graphics.CreateIndexBuffer(ibo, IndexData.data(), (uint32)(IndexData.size() * sizeof(uint32)), false);

	TempFormater formater;
	context.Graphics.SetVertexBufferName(ibo, formater.Format("{} VB", path));
	context.Graphics.SetIndexBufferName(ibo, formater.Format("{} IB", path));

	GPUGeometry meshGeometry;
	meshGeometry.Vbo = vbo;
	meshGeometry.Ibo = ibo;
	meshGeometry.Description.VertexCount = (uint32)VertexData.size();
	meshGeometry.Description.IndexCount = (uint32)IndexData.size();
	meshGeometry.Description.Topology = TT_TRIANGLES;

	Mesh mesh;
	mesh.Geometry = meshGeometry;
	mesh.Material = 1;

	context.Renderer3D.MeshData.Meshes.insert({nextMesh, mesh});

	context.Meshes.push_back(MeshEditEntry{nextMesh});

	context.MeshNames.push_back(path);
}
