#include "AssetBuilder.hpp"

#include <sstream>

struct  WavHeader
{
	/* RIFF Chunk Descriptor */
	uint8_t         RIFF[4];        // RIFF Header Magic header
	uint32_t        ChunkSize;      // RIFF Chunk Size
	uint8_t         WAVE[4];        // WAVE Header
	/* "Fmt" sub-chunk */
	uint8_t         Fmt[4];         // FMT header
	uint32_t        Subchunk1Size;  // Size of the Fmt chunk
	uint16_t        AudioFormat;    // Audio format 1=PCM,6=mulaw,7=alaw,     257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
	uint16_t        NumOfChan;      // Number of channels 1=Mono 2=Sterio
	uint32_t        SamplesPerSec;  // Sampling Frequency in Hz
	uint32_t        BytesPerSec;    // bytes per second
	uint16_t        BlockAlign;     // 2=16-bit mono, 4=16-bit stereo
	uint16_t        BitsPerSample;  // Number of bits per sample
	/* "data" sub-chunk */
	uint8_t         Subchunk2ID[4]; // "data"  string
	uint32_t        Subchunk2Size;  // Sampled data length
};

// @Note: Taken from the al.h header of OpenAL
/** Unsigned 8-bit mono buffer format. */
#define AL_FORMAT_MONO8                          0x1100
/** Signed 16-bit mono buffer format. */
#define AL_FORMAT_MONO16                         0x1101
/** Unsigned 8-bit stereo buffer format. */
#define AL_FORMAT_STEREO8                        0x1102
/** Signed 16-bit stereo buffer format. */
#define AL_FORMAT_STEREO16                       0x1103

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

void LoadImage(AssetToLoad asset, AssetBundlerContext& context, AssetDataBlob& blob)
{
	int width, height, channels;
	unsigned char* data = stbi_load(asset.Path.c_str(), &width, &height, &channels, 4);

	ImageLoadEntry imageEntry{ 0 };
	imageEntry.Desc.Width = width;
	imageEntry.Desc.Height = height;
	imageEntry.Desc.Format = TF_RGBA;
	imageEntry.Id = NewAssetName(context, Type_Image, asset.Id);
	imageEntry.DataOffset = blob.PutData(data, width * height * channels);

	context.LoadImages.push_back(imageEntry);
	
	stbi_image_free(data);
}

void LoadWav(AssetToLoad asset, AssetBundlerContext& context, AssetDataBlob& blob)
{
	auto data = LoadFile(asset.Path);
	WavHeader* wavHeader = (WavHeader*)(data.data());

	WavLoadEntry wavEntry;
	wavEntry.Desc.Size = wavHeader->Subchunk2Size;
	wavEntry.Desc.SampleRate = wavHeader->SamplesPerSec;
	wavEntry.Desc.Channels = wavHeader->NumOfChan;
	wavEntry.Desc.Bps = wavHeader->BitsPerSample;
	wavEntry.Id = NewAssetName(context, Type_Wav, asset.Id);

	if (wavEntry.Desc.Channels == 1)
	{
		if (wavEntry.Desc.Bps == 8)
		{
			wavEntry.Desc.Format = AL_FORMAT_MONO8;
		}
		else
		{
			wavEntry.Desc.Format = AL_FORMAT_MONO16;
		}
	}
	else
	{
		if (wavEntry.Desc.Bps == 8)
		{
			wavEntry.Desc.Format = AL_FORMAT_STEREO8;
		}
		else
		{
			wavEntry.Desc.Format = AL_FORMAT_STEREO16;
		}
	}
	
	wavEntry.DataOffset = blob.PutData(data, sizeof(WavHeader));	
	context.LoadWavs.push_back(wavEntry);
}

void LoadFont(AssetToLoad asset, AssetBundlerContext& context, AssetDataBlob& blob)
{
	auto data = LoadFile(asset.Path);
	
	FontLoadEntry fontEntry{0};
	fontEntry.Desc.FontSize = 24;
	fontEntry.Id = NewAssetName(context, Type_Font, fmt::format("{}_{}", asset.Id, asset.data.unsigned1).c_str());
	
	fontEntry.DataSize = (uint32)data.size();
	fontEntry.DataOffset = blob.PutData(data);

	context.LoadFonts.push_back(fontEntry);
}

void LoadTexture(AssetToLoad asset, AssetBundlerContext& context, AssetDataBlob& blob)
{
	int width, height, channels;
	unsigned char* data = stbi_load(asset.Path.c_str(), &width, &height, &channels, 4);

	TextureLoadEntry texEntry;
	texEntry.Id = NextTextureAssetId();
	texEntry.Desc.Width = width;
	texEntry.Desc.Height = height;
	texEntry.Desc.Format = TF_RGBA;
	texEntry.DataOffset = blob.PutData(data, width*height*channels);
		
	context.TexturesToCreate.push_back(texEntry);
	
	NewAssetName(context, Type_Texture, asset.Id, texEntry.Id);

	stbi_image_free(data);
}

void LoadSkybox(AssetToLoad asset, AssetBundlerContext& context, AssetDataBlob& blob)
{
	std::string paths[] = {
		fmt::format("{}/left.png", asset.Path.c_str()),
		fmt::format("{}/right.png", asset.Path.c_str()),
		fmt::format("{}/up.png", asset.Path.c_str()),
		fmt::format("{}/down.png", asset.Path.c_str()),
		fmt::format("{}/front.png", asset.Path.c_str()),
		fmt::format("{}/back.png", asset.Path.c_str()),
	};

	for (size_t i = 0; i < 6; ++i)
	{
		if (!fs::is_regular_file(paths[i]))
		{
			paths[i] = fs::path(paths[i]).replace_extension("jpg").string();
		}

	}
	SkyboxLoadEntry skybox;
	skybox.Id = NextTextureAssetId();

	NewAssetName(context, Type_Texture, asset.Id, skybox.Id);
	
	for (size_t i = 0; i < 6; ++i)
	{
		stbi_set_flip_vertically_on_load(0);
		int width, height, channels;
		unsigned char* data = stbi_load(paths[i].c_str(), &width, &height, &channels, 4);

		skybox.Desc.Width = width;
		skybox.Desc.Height = height;
		skybox.Desc.Format = TF_RGBA;
		skybox.DataOffset[i] = blob.PutData(data, width*height*4);

		stbi_image_free(data);
	}

	context.Skyboxes.push_back(skybox);

}

static void LoadMaterial(fs::path path, AssetBundlerContext& context, AssetDataBlob& blob)
{
	auto content = LoadFileIntoString(path.string());
	std::stringstream stream(content);
	std::string line;

	std::string newMatName;

	MtlMaterial newMat{0};

	const ShaderConfig configs[] = {
		SC_MTL_1,
		SC_MTL_2
	};
	
	while(std::getline(stream, line, '\n'))
	{
		if (line[0] == '#') continue;

		if (StartsWith(line, "newmtl"))
		{
			auto parts = SplitLine(line, ' ');
			newMatName = ReplaceAll(parts[1], ".", "_");
		}
		else if (StartsWith(line, "Ns"))
		{
			auto parts = SplitLine(line, ' ');
			newMat.Ns = std::stof(parts[1].c_str());
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
		}
		else if (StartsWith(line, "illum"))
		{
			auto parts = SplitLine(line, ' ');
			newMat.illum = (uint32)std::stoi(parts[1].c_str());
			assert(newMat.illum <= 2);
			newMat.Program = configs[newMat.illum - 1];
		}
		else if (StartsWith(line, "Ka"))
		{
			auto parts = SplitLine(line, ' ');
			newMat.Ka = {
				std::stof(parts[1].c_str()),
				std::stof(parts[2].c_str()),
				std::stof(parts[3].c_str())
			};
		}
		else if (StartsWith(line, "Kd"))
		{
			auto parts = SplitLine(line, ' ');
			newMat.Kd = {
				std::stof(parts[1].c_str()),
				std::stof(parts[2].c_str()),
				std::stof(parts[3].c_str())
			};
		}
		else if (StartsWith(line, "Ks"))
		{
			auto parts = SplitLine(line, ' ');
			newMat.Ks = {
				std::stof(parts[1].c_str()),
				std::stof(parts[2].c_str()),
				std::stof(parts[3].c_str())
			};
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
			AssetToLoad texAsset;
			texAsset.Path = ReplaceAll(line,"map_Kd ", "");
			texAsset.Id = "Kd_Map";
			LoadTexture(texAsset, context, blob);
			newMat.KdMap = context.TexturesToCreate.back().Id;	
		}
		else if (StartsWith(line, "map_Ka"))
		{
			AssetToLoad texAsset;
			texAsset.Path = ReplaceAll(line,"map_Ka ", "");
			texAsset.Id = "Ka_Map";
			LoadTexture(texAsset, context, blob);
			newMat.KaMap = context.TexturesToCreate.back().Id;	
		}
		
	}

	MaterialLoadEntry newEntry;
	newEntry.Desc = newMat;
	newEntry.Id = NewAssetName(context, Type_Material, newMatName.c_str());
	context.Materials.push_back(newEntry);
}
	
void LoadMesh(AssetToLoad asset, AssetBundlerContext& context, AssetDataBlob& blob)
{
	auto content = LoadFileIntoString(asset.Path);
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

	VBLoadEntry vbo;
	IBLoadEntry ibo;
	MeshLoadEntry mesh;

	while(std::getline(stream, line, '\n'))
	{
		if (line[0] == '#') continue;
		
		if (StartsWith(line, "mtllib"))
		{
			 auto parts = SplitLine(line, ' ');
			 auto materialPath = fs::path(asset.Path).parent_path() / fs::path(parts[1]);
			 LoadMaterial(materialPath, context, blob);
		}
		else if (line[0] == 'v' && line[1] == ' ')
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
			
		}
		else if (StartsWith(line, "usemtl"))
		{
			auto parts = SplitLine(line, ' ');
			auto matName = ReplaceAll(parts[1], ".", "_");
			auto id = std::find_if(context.Defines.begin(), context.Defines.end(), [&matName](auto def) { return def.Name == matName; });
			assert(id != context.Defines.end());
			mesh.Mesh.Material = id->Id;			
		}
    }

	vbo.StructSize = sizeof(MtlVertex);
	vbo.DataSize = (uint32)(sizeof(MtlVertex) * VertexData.size());
	vbo.Dynamic = false;
	vbo.DataOffset = blob.PutData((unsigned char*)VertexData.data(), sizeof(MtlVertex) * VertexData.size());
	vbo.Id = NextVBAssetId();

	ibo.DataSize = (uint32)(sizeof(uint32) * IndexData.size());
	ibo.Dynamic = false;
	ibo.DataOffset = blob.PutData((unsigned char*)IndexData.data(), sizeof(uint32) * IndexData.size());
	ibo.Id = NextIBAssetId();
	
	mesh.Mesh.Geometry.VertexBuffer= vbo.Id;
	mesh.Mesh.Geometry.IndexBuffer = ibo.Id;
	mesh.Mesh.Geometry.IndexCount = (uint32)IndexData.size();
	mesh.Id = NewAssetName(context, Type_Mesh, asset.Id);

	context.VBsToCreate.push_back(vbo);
	context.IBsToCreate.push_back(ibo);
	context.LoadMeshes.push_back(mesh);
}
