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
