#pragma once


#include <Platform.hpp>
#include <Memory.hpp>
#include <Types.hpp>
#include <Logging.hpp>


inline void ReadWholeFile(const char* t_Path, MemoryArena& t_Arena)
{
	auto handle = PlatformLayer::OpenFileForReading(t_Path);
	auto fileSize = PlatformLayer::FileSize(handle);

	Assert(fileSize < t_Arena.MaxSize, "Can't read the whole file into the given arena: {}", t_Path);
	
	PlatformLayer::ReadFileIntoArena(handle, fileSize, t_Arena);

	PlatformLayer::CloseFile(handle);
}
