#pragma once

#include "Utils.hpp"
#include "PlatformWindows.hpp"

#include <assert.h>

struct MemoryArena
{
	char* Memory;
	char* Current;
	size_t MaxSize;
	size_t Size;

	void Put(void* data, size_t len)
	{
		assert(Size + len <= MaxSize);
		memcpy(Memory + Size, data, len);
		Current += len;
		Size += len;
	}

	template<typename T = void>
	T* Get(size_t len)
	{
		assert(Size + len <= MaxSize);
		Size += len;
		Current += len;
		return (T*)(Memory + Size);
	}

	void Reset()
	{
		Size = 0;
		Current = Memory;
	}

};

struct MemoryState
{
	char* TempMemory;
	char* TempMemoryCurrent;
	size_t TempMemoryMaxSize;
	size_t TempMemorySize;
	
	char* BulkMemory;
	size_t BulkMemorySize;

};

struct Memory
{
	const static size_t TempMemoryRequired;
	const static size_t BulkMemoryRequired;
	const static size_t TotalMemoryRequired;
	static MemoryState g_Memory;
	static MemoryArena g_TempScope;
	

	// @Note: Get some storage in arena form, use it and then
	// give it back
	static MemoryArena GetTempArena(size_t t_Size);
	static void DestoryTempArena(MemoryArena& t_Arena);

	// @Note: Establish global temporary arena that will be used for
	// TempAlloc\TempRealoc; those can them be used are general allocators
	// but you should be mindfull of the temporary aspect
	//   -> Linear allocation
	//   -> Limited reallocations
	//   -> There are no deallocations
	static void EstablishTempScope(size_t t_Size);
	static void* TempAlloc(size_t len);
	static void* TempRealloc(void* mem, size_t len);
	static void ReleaseTempScope();

    //  @Note: Reset the current temp global temp scope arena
	// to its initial state 
	static void ResetTempScope();

	// @Note: Reset the whole temporary memory
	// in one big go; all arenas should be treated
	// as invalid after that
	static void ResetTempMemory();

	// @Note: Initalize the whole memory by requesting memory from the OS
	static void InitMemoryState();
};

inline void ReadWholeFile(const char* t_Path, MemoryArena& t_Arena)
{

	HANDLE  handle = CreateFile(t_Path, GENERIC_READ, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD fileSize = GetFileSize(handle, NULL);
	DWORD readBytes;

	assert(fileSize < t_Arena.MaxSize);
	ReadFile(handle, t_Arena.Memory + t_Arena.Size, fileSize, &readBytes, NULL);

	t_Arena.Size += readBytes;

	CloseHandle(handle);
}
