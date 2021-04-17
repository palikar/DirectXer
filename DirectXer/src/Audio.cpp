#pragma once

#include "Audio.hpp"

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

static char* LoadWAV(char* memory, int& chan, int& samplerate, int& bps, int& size)
{
	auto current = memory;
	auto header = ReadBlobAndMove<WavHeader>(current, sizeof(WavHeader));
	chan = header.NumOfChan;
	samplerate = header.SamplesPerSec;
	bps = header.BitsPerSample;
	size = header.Subchunk2Size;
	return current;
}

static int InitAL()
{
	const ALCchar* name;
	ALCdevice* device;
	ALCcontext* ctx;

	/* Open and initialize a device */
	device = NULL;
	device = alcOpenDevice(NULL);
	if (!device)
	{
		fprintf(stderr, "Could not open a device!\n");
		return 1;
	}

	ctx = alcCreateContext(device, NULL);
	if (ctx == NULL || alcMakeContextCurrent(ctx) == ALC_FALSE)
	{
		if (ctx != NULL)
			alcDestroyContext(ctx);
		alcCloseDevice(device);
		fprintf(stderr, "Could not set a context!\n");
		return 1;
	}

	name = NULL;
	if (alcIsExtensionPresent(device, "ALC_ENUMERATE_ALL_EXT"))
		name = alcGetString(device, ALC_ALL_DEVICES_SPECIFIER);
	if (!name || alcGetError(device) != AL_NO_ERROR)
		name = alcGetString(device, ALC_DEVICE_SPECIFIER);
	printf("Opened \"%s\"\n", name);

	return 0;
}

void Audio::Init()
{
	InitAL();
}

void AudioBuilder::Init(uint16 t_WavCount)
{
	QueuedWavs.reserve(t_WavCount);
	MaxFileSize = 0;
}

uint32 AudioBuilder::PutWav(std::string_view t_Path)
{
	auto path = Resources::ResolveFilePath(t_Path);
	QueuedWav newWav;
	newWav.Handle = PlatformLayer::OpenFileForReading(path);
	newWav.Path = t_Path;
	newWav.FileSize = PlatformLayer::FileSize(newWav.Handle);
	QueuedWavs.push_back(newWav);

	MaxFileSize = MaxFileSize < newWav.FileSize ? newWav.FileSize  : MaxFileSize;

	return (uint32)QueuedWavs.size() - 1;
}

void AudioPlayer::Build(AudioBuilder& t_Builder)
{
	MemoryArena fileArena = Memory::GetTempArena(t_Builder.MaxFileSize + Megabytes(1));
	Memory::EstablishTempScope(Megabytes(64));
	Defer {
		Memory::EndTempScope();
		Memory::DestoryTempArena(fileArena);
	};

	for (auto& entry : t_Builder.QueuedWavs)
	{
		Defer {
			fileArena.Reset();
			Memory::ResetTempScope();
		};

		PlatformLayer::ReadFileIntoArena(entry.Handle, entry.FileSize, fileArena);

		int channel, sampleRate, bps, size;
		auto data = LoadWAV(fileArena.Memory, channel, sampleRate, bps, size);

		unsigned int bufferid, format;
		alGenBuffers(1, &bufferid);
		if (channel == 1)
		{
			if (bps == 8)
			{
				format = AL_FORMAT_MONO8;
			}
			else {
				format = AL_FORMAT_MONO16;
			}
		}
		else
		{
			if (bps == 8)
			{
				format = AL_FORMAT_STEREO8;
			}
			else {
				format = AL_FORMAT_STEREO16;
			}
		}
		alBufferData(bufferid, format, data, size, sampleRate);
		unsigned int sourceid;
		alGenSources(1, &sourceid);
		alSourcei(sourceid, AL_BUFFER, bufferid);

		AudioEntries.push_back({bufferid, sourceid});

	}
}

void AudioPlayer::Play(uint32 t_Id, float t_Gain)
{
	const auto source =  AudioEntries[t_Id].Source;
	alSourcef(source, AL_GAIN, t_Gain);
	alSourcePlay(source);
}

void AudioPlayer::CreateMemoryWav(WavAssetHeader& header, void* data)
{
	unsigned int bufferid;
	unsigned int sourceid;

	alGenBuffers(1, &bufferid);
	alBufferData(bufferid, header.Format, data, header.Size, header.SampleRate);
	alGenSources(1, &sourceid);
	alSourcei(sourceid, AL_BUFFER, bufferid);

	AudioEntries.push_back({bufferid, sourceid});
}
