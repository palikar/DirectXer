#pragma once

#include <Memory.hpp>
#include <Resources.hpp>
#include <Fileutils.hpp>
#include <Assets.hpp>

#include <AL/al.h>
#include <AL/alext.h>
#include <AL/alext.h>


struct Audio
{
	static void Init();
};

struct AudioBuilder
{
	struct QueuedWav
	{
		String Path;
		PlatformLayer::FileHandle Handle;
		size_t FileSize;
	};

	TempVector<QueuedWav> QueuedWavs;
	size_t MaxFileSize;

	void Init(uint16 t_WavCount);
	uint32 PutWav(std::string_view t_Path);
};

class AudioPlayer
{
  public:
	
	struct AudioEntry
	{
		unsigned Buffer;
		unsigned Source;
	};

	BulkVector<AudioEntry> AudioEntries;

	void Build(AudioBuilder& t_Builder);

	void CreateMemoryWav(WavAssetHeader& header, void* data)
	{
		unsigned int bufferid;
		unsigned int sourceid;

		alGenBuffers(1, &bufferid);
		alBufferData(bufferid, header.Format, data, header.Size, header.SampleRate);
		alGenSources(1, &sourceid);
		alSourcei(sourceid, AL_BUFFER, bufferid);

		AudioEntries.push_back({bufferid, sourceid});
	}
	
	void Play(uint32 t_Id, float t_Gain);
};
