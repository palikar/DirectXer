#include "AssetBuilder.hpp"

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

// @Note: These functions hould produce some data based on the given asset
// to load; the data should be *appended* to the end of the bytes vector and
// the amount of new data should be returned; the appended data can contain
// (and probably should) some sort of header discribing the asset which's data
// should come after the header

size_t LoadImage(AssetToLoad asset, std::vector<unsigned char>& bytes, uint32 id)
{
	int width, height, channels;
	unsigned char* data = stbi_load(asset.Path.c_str(), &width, &height, &channels, 4);

	ImageHeader header{ 0 };
	header.Width = width;
	header.Height = height;
	header.Format = TF_RGBA;
	header.Id = id;

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
	if (header.Channels == 1)
	{
		if (header.Bps == 8)
		{
			header.Format = AL_FORMAT_MONO8;
		}
		else {
			header.Format = AL_FORMAT_MONO16;
		}
	}
	else
	{
		if (header.Bps == 8)
		{
			header.Format = AL_FORMAT_STEREO8;
		}
		else {
			header.Format = AL_FORMAT_STEREO16;
		}
	}

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

size_t LoadFont(AssetToLoad asset, std::vector<unsigned char>& bytes, uint32 id)
{
	auto data = LoadFile(asset.Path);
	
	FontHeader header{0};
	header.FontSize = 24;
	header.Id = id;
	header.DataSize = (uint32)data.size();
	
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

size_t LoadAtlas(AssetToLoad asset, std::vector<unsigned char>& bytes,
				 std::vector<std::string>& headerDefines, AssetColletionHeader& header)
{
	const auto atlasData = LoadFile(asset.Path);

	const AtlasFileHeader* atlasHeader = (AtlasFileHeader*) atlasData.data();
	const size_t offset = sizeof(AtlasFileHeader) + sizeof(AtlasEntry) * atlasHeader->NumAtlases;

	for (size_t i = 0; i < atlasHeader->NumImages; ++i)
	{
		ImageEntry* image = (ImageEntry*)(atlasData.data() + i * sizeof(ImageEntry) + offset);
		headerDefines.push_back(fmt::format("#define {}\t{}", image->Name, header.AtlasesCount << 16 | i));
		image->Id = (uint32)(header.AtlasesCount << 16 | i);
	}

	for (auto b : atlasData)
	{
		bytes.push_back(b);
	}

	return atlasData.size();
}
