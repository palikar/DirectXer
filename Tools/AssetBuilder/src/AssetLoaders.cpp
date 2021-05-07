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

	SkyboxLoadEntry skybox;
	skybox.Id = NextTextureAssetId();

	NewAssetName(context, Type_Texture, asset.Id, skybox.Id);
	
	for (size_t i = 0; i < 6; ++i)
	{
		stbi_set_flip_vertically_on_load(1);
		int width, height, channels;
		unsigned char* data = stbi_load(paths[i].c_str(), &width, &height, &channels, 4);

		skybox.Desc.Width = width;
		skybox.Desc.Height = height;
		skybox.Desc.Format = TF_RGBA;
		skybox.DataOffset[i] = blob.PutData(data, width*height*channels);

		stbi_image_free(data);
	}

	context.Skyboxes.push_back(skybox);

}

void LoadMesh(AssetToLoad asset, AssetBundlerContext& context, AssetDataBlob& blob)
{
	auto content = LoadFileIntoString(asset.Path);
	std::stringstream stream(content);
	std::string line;

	std::vector<ColorVertex> VertexData;
	std::vector<uint32> IndexData;

	std::vector<glm::vec3> Pos;
	std::vector<glm::vec3> Norms;
	std::vector<glm::vec2> UVs;

	std::unordered_map<std::string, uint32> indexMap;

	while(std::getline(stream, line, '\n'))
	{
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
			
			const auto i = (uint32) VertexData.size();

			ColorVertex nextVertex;
			if (indexMap.insert({parts[1], i}).second)
			{
				nextVertex.pos = Pos[vtn1.x];
				nextVertex.normal = Norms[vtn1.z];
				nextVertex.uv = UVs[vtn1.y];
				VertexData.push_back(nextVertex);
			}
			
			if (indexMap.insert({parts[2], i + 1}).second)
			{
				nextVertex.pos = Pos[vtn2.x];
				nextVertex.normal = Norms[vtn2.z];
				nextVertex.uv = UVs[vtn2.y];
				VertexData.push_back(nextVertex);
			}
			
			if (indexMap.insert({parts[3], i + 2}).second)
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
    }

	MeshLoadEntry mesh;

	mesh.VBDesc.StructSize = sizeof(ColorVertex);
	mesh.VBDesc.DataSize = (uint32)(sizeof(ColorVertex) * VertexData.size());
	mesh.IBDesc.StructSize = 0;
	mesh.IBDesc.DataSize = (uint32)(sizeof(uint32) * IndexData.size());
	
	mesh.Vbo = NextVBAssetId();
	mesh.Ibo = NextIBAssetId();

	mesh.DataOffsetVBO = blob.PutData((unsigned char*)VertexData.data(), sizeof(ColorVertex) * VertexData.size());
	mesh.DataOffsetIBO = blob.PutData((unsigned char*)IndexData.data(), sizeof(uint32) * IndexData.size());

	context.LoadMeshes.push_back(mesh);

	NewAssetName(context, Type_Mesh, asset.Id, 45);
}
