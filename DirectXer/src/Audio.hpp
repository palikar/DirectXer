#pragma once

#include <Memory.hpp>
#include <Resources.hpp>
#include <Fileutils.hpp>

#include <AL/al.h>
#include <AL/alext.h>
#include <AL/alext.h>

using WavId = uint32;
	
struct WavDescription
{
	uint32 Size;
	uint32 SampleRate;
	uint32 Format;
	uint16 Channels;
	uint16 Bps;
};

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
	void CreateMemoryWav(WavDescription& header, void* data);
	void Play(uint32 t_Id, float t_Gain);
};
