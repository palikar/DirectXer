#pragma once


#include "Platform.hpp"
#include "Memory.hpp"
#include "Types.hpp"


inline void ReadWholeFile(const char* t_Path, MemoryArena& t_Arena)
{
	auto handle = PlatformLayer::OpenFileForReading(t_Path);
	auto fileSize = PlatformLayer::FileSize(handle);

	assert(fileSize < t_Arena.MaxSize);
	
	PlatformLayer::ReadFileIntoArena(handle, fileSize, t_Arena);
	t_Arena.Size += fileSize;

	PlatformLayer::CloseFile(handle);
}
