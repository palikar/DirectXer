#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>
#include <fstream>
#include <stb_image.h>
#include <GraphicsCommon.hpp>
#include <Types.hpp>
#include <Utils.hpp>
#include <Assets.hpp>
#include <fmt/format.h>
#include <filesystem>

// ================================================================================

void* TempAlloc(size_t size)
{
	return malloc(size);
}

void* TempRealloc(void* mem, size_t size)
{
	return realloc(mem, size);
}

void* TempFree(void* mem)
{
	free(mem);
	return nullptr;
}

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

std::vector<unsigned char> LoadFile(const std::string &t_filename)
{
    std::ifstream infile(t_filename.c_str(), std::ios::in | std::ios::ate | std::ios::binary);

	auto size = infile.tellg();
    infile.seekg(0, std::ios::beg);

	std::vector<unsigned char> v(static_cast<size_t>(size));
	infile.read((char*)&v[0], static_cast<std::streamsize>(size));

	return v;
}

// ================================================================================

struct ExtraData
{
	uint32 unsigned1;
	uint32 unsigned2;
	int integet1;
	int integet2;
	float float1;
	float float2;
};

struct AssetToLoad
{
	AssetType Type;
	Tag TagField;
	std::string Path;
	const char* Id;
	ExtraData data{0};
};

static AssetToLoad AssetsToLoad[] = {

	{Type_Image, Tag_Level, "assets/PNG/Main_Menu/BG.png", "I_BG"},
	{Type_Image, Tag_Level, "assets/PNG/Ship_Parts/Ship_Main_Icon.png", "I_SHIP"},
	{Type_Image, Tag_Level, "assets/evil_ship_1.png", "I_ENEMY_1"},
	{Type_Image, Tag_Level, "assets/evil_ship_2.png", "I_ENEMY_2"},
	{Type_Image, Tag_Level, "assets/evil_ship_3.png", "I_ENEMY_3"},
	{Type_Image, Tag_Level, "assets/ship_bullet.png", "i_BULLET"},
	{Type_Image, Tag_Level, "assets/explosion.png", "i_EXPLODE"},
	{Type_Image, Tag_Level, "assets/PNG/Main_UI/Stats_Bar.png", "I_STATS"},
	{Type_Image, Tag_Level, "assets/PNG/Main_UI/Health_Bar_Table.png", "i_HEALTH"},
	{Type_Image, Tag_Level, "assets/heart.png", "I_HEART"},

	{Type_Font, Tag_Level, "fonts/DroidSans/DroidSans.ttf", "F_DroidSans", {24, 20}},
	{Type_Font, Tag_Level, "fonts/DroidSans/DroidSans-Bold.ttf", "F_DroidSansBold", {24, 20}},
	
	{Type_Wav, Tag_Level,  "shoot.wav", "A_SHOOT"},
	{Type_Wav, Tag_Level,  "explosion.wav", "A_EXPLODE"},
};

// ================================================================================

size_t LoadImage(AssetToLoad asset, std::vector<unsigned char>& bytes)
{
	int width, height, channels;
	unsigned char* data = stbi_load(asset.Path.c_str(), &width, &height, &channels, 4);

	ImageHeader header{ 0 };
	header.Width = width;
	header.Height = height;
	header.Format = TF_RGBA;

	for (size_t i = 0; i < sizeof(ImageHeader); ++i)
	{
		bytes.push_back(*((char*)&header + i));
	}
	
	for (size_t i = 0; i < width*height*channels; ++i)
	{
		bytes.push_back(data[i]);
	}

	stbi_image_free(data);

	return width * height * channels + sizeof(ImageHeader);
}

size_t LoadWav(AssetToLoad asset, std::vector<unsigned char>& bytes)
{
	auto data = LoadFile(asset.Path);
	WavHeader* wavHeader = (WavHeader*)(data.data());

	WavAssetHeader header;
	header.Size = wavHeader->Subchunk2Size;
	header.SampleRate = wavHeader->SamplesPerSec;
	header.Channels = wavHeader->NumOfChan;
	header.Bps = wavHeader->BitsPerSample;

	for (size_t i = 0; i < sizeof(WavHeader); ++i)
	{
		bytes.push_back(*((char*)&header + i));
	}
		
	for (size_t i = 0; i < wavHeader->Subchunk2Size; ++i)
	{
		bytes.push_back(data[sizeof(WavHeader) + i]);
	}

	return wavHeader->Subchunk2Size + sizeof(WavHeader);
}

size_t LoadFont(AssetToLoad asset, std::vector<unsigned char>& bytes)
{
	auto data = LoadFile(asset.Path);
	
	FontHeader header{0};
	header.FontSize = 24;
	
	for (size_t i = 0; i < sizeof(FontHeader); ++i)
	{
		bytes.push_back(*((char*)&header + i));
	}
		
	for (size_t i = 0; i < data.size(); ++i)
	{
		bytes.push_back(data[i]);
	}

	return data.size() + sizeof(FontHeader);
}

// ================================================================================


struct CommandLineArguments
{
	std::string Root{""};
	std::string Output{"output"};
	std::string Header{"output"};
	std::string Id{"Asset"};
	size_t MaxSize{128};
};

void ParseCommandLineArguments(int argc, char *argv[], CommandLineArguments& arguments);

template<class T, size_t N>
constexpr size_t size(T (&)[N])
{
	return N;
}

int main(int argc, char *argv[])
{
	CommandLineArguments arguments{};
	ParseCommandLineArguments(argc, argv, arguments);

	AssetColletionHeader header{ 0 };
	header.VersionSpec = 1;

	std::vector<AssetEntry> entries;
	entries.reserve(size(AssetsToLoad));
	
	std::vector<unsigned char> data;
	data.reserve(Megabytes(arguments.MaxSize));

	std::vector<std::string> headerDefines;
	
	size_t currentOffset = size(AssetsToLoad) * sizeof(AssetEntry) + sizeof(AssetColletionHeader);
	
	for (size_t i = 0; i < size(AssetsToLoad); ++i)
	{
		auto& asset = AssetsToLoad[i];
		asset.Path = fmt::format("{}/{}", arguments.Root, asset.Path);
		size_t size = 0;
		
		if(asset.Type == Type_Image)
		{
			headerDefines.push_back(fmt::format("#define {}\t{}", asset.Id, header.ImagesCount));
			header.ImagesCount += 1;
			size = LoadImage(asset, data);
			if(size > 1024*1024)
			{
				std::cout << fmt::format("{} -> Packing image [{:.3} MB] [{}]\n", asset.Id, size/(1024.0f*1024.0f), asset.Path); 
			}
			else
			{
				std::cout << fmt::format("{} -> Packing image [{:.3} KB] [{}]\n", asset.Id, size/1024.0f, asset.Path); 
			}
		}
		else if(asset.Type == Type_Font)
		{
			headerDefines.push_back(fmt::format("#define {}\t{}", asset.Id, header.FontsCount));
			header.FontsCount += 1;
			size = LoadFont(asset, data);
			std::cout << fmt::format("{} -> Packing font [{:.3} KB] [{}]\n", asset.Id, size/1024.0f, asset.Path);
		}
		else if(asset.Type == Type_Wav)
		{
			headerDefines.push_back(fmt::format("#define {}\t{}", asset.Id, header.WavCount));
			header.WavCount += 1;
			size = LoadWav(asset, data);
			std::cout << fmt::format("{} -> Packing WAV [{:.3} KB] [{}]\n", asset.Id, size/1024.0f, asset.Path);
		}
		
		AssetEntry entry{ 0 };
		entry.Offset = currentOffset;
		entry.Size = size;
		entry.Type = asset.Type;
		entry.Tag = asset.TagField;
		entries.push_back(entry);

		currentOffset += size;
	}

	auto size = sizeof(AssetColletionHeader) + entries.size() * sizeof(AssetEntry) + data.size();
	std::cout << fmt::format("Creating file [{:.3} MB] [{}.dx1]\n",size/(1024.0f*1024.0f), arguments.Output);

	std::string assetFileName = fmt::format("{}.dx1", arguments.Output); 
	std::ofstream outfile(assetFileName, std::ios::out | std::ios::binary);

	outfile.write((char*)&header, sizeof(AssetColletionHeader));
	outfile.write((char*)entries.data(), entries.size() * sizeof(AssetEntry));
	outfile.write((char*)data.data(), data.size());

	outfile.flush();
	outfile.close();

	std::cout << fmt::format("Creating file [{}.hpp] with [{}] defines\n", arguments.Header, headerDefines.size());
		
	std::ofstream headerFile(fmt::format("{}.hpp", arguments.Header), std::ios::out);
	headerFile << "#pragma once\n";

	headerFile << "\n\n";
	for(auto& define : headerDefines)
	{
		headerFile << define << "\n";
	}
	headerFile << "\n\n";
	
	headerFile << "static inline const char* ImageAssets[] = {\n";
	for(auto& asset : AssetsToLoad)
		if(asset.Type == Type_Image) headerFile << fmt::format("\t\"{}\",\n", asset.Id);
	headerFile << "};\n\n";

	headerFile << "static inline const char* FontAssets[] = {\n";
	for(auto& asset : AssetsToLoad)
		if(asset.Type == Type_Font) headerFile << fmt::format("\t\"{}\",\n", asset.Id);
	headerFile << "};\n\n";

	headerFile << "static inline const char* WavAssets[] = {\n";
	for(auto& asset : AssetsToLoad)
		if(asset.Type == Type_Wav) headerFile << fmt::format("\t\"{}\",\n", asset.Id);
	headerFile << "};\n\n";

	headerFile << fmt::format("static inline const char* {}AssetFilePath = \"{}\";", arguments.Id, assetFileName);
	headerFile << "\n\n";
	
    return 0;
}

void ParseCommandLineArguments(int argc, char *argv[], CommandLineArguments& arguments)
{
	for (size_t i = 0; i < argc; ++i)
	{
		std::string current{argv[i]};
		if(current == "-r") {
			arguments.Root = argv[++i];
		} else if (current == "-o") {
			arguments.Output = argv[++i];
		} else if (current == "-m") {
			arguments.MaxSize = std::stoi(argv[++i]) * 1024 * 1024;
		} else if (current == "-h") {
			arguments.Header = argv[++i];
		} else if (current == "-i") {
			arguments.Id = argv[++i];
		}
	}
}


// @Todo: Texture Loading
// @Todo: Materials Loading
// @Todo: Objects Loading

// @Todo: Asset bundle description file
