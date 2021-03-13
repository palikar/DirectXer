#pragma once

#include "Utils.hpp"
#include "PlatformWindows.hpp"

#include <vector>
#include <string>

#include <assert.h>

struct MemoryArena
{
	char* Memory;
	char* Current;
	size_t MaxSize;
	size_t Size;

	void Put(const void* data, size_t len)
	{
		assert(Size + len <= MaxSize);
		memcpy(Memory + Size, data, len);
		Current += len;
		Size += len;
	}

	template<typename T>
	void Put(const T& value)
	{
		Put(&value, sizeof(T));
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
	char* BulkMemoryCurrent;
	size_t BulkMemorySize;
	size_t BulkMemoryMaxSize;

};

struct Memory
{
	const static size_t TempMemoryRequired;
	const static size_t BulkMemoryRequired;
	const static size_t TotalMemoryRequired;
	static MemoryState g_Memory;
	static MemoryArena g_TempScope;

	static void* BulkGet(size_t t_Size);

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
	static void TempDealloc(void*);

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


template<typename T>
class TempStdAllocator
{
  public:

	TempStdAllocator(){};
	
	TempStdAllocator(const TempStdAllocator&){};
	
	typedef T value_type;
	typedef size_t size_type;
	typedef std::ptrdiff_t difference_type;
	typedef std::true_type is_always_equal;

	T* allocate(size_type t_Size)
	{
		return (T*)Memory::TempAlloc(sizeof(T) * t_Size);
	}
	
	void deallocate(T* p, size_type)
	{
		return Memory::TempDealloc(p);
	}

	inline bool operator==(TempStdAllocator const&) const { return true; }
		
};

template<typename T>
class BulkStdAllocator
{
  public:

	BulkStdAllocator(){};
	
	BulkStdAllocator(const BulkStdAllocator&){};

	
	typedef T value_type;
	typedef size_t size_type;
	typedef std::ptrdiff_t difference_type;
	typedef std::true_type is_always_equal;

	T* allocate(size_type t_Size)
	{
		return (T*)Memory::BulkGet(sizeof(T) * t_Size);
	}
	
	void deallocate(T*, size_type){}

	inline bool operator==(BulkStdAllocator const&) const { return true; }
		
};


namespace asl
{

// template<class T>
// using TempVector = std::vector<T, TempStdAllocator<T>>;
// using TempString = std::basic_string<char, std::char_traits<char>, TempStdAllocator<char>>;
// using TempWString = std::basic_string<wchar_t, std::char_traits<wchar_t>, TempStdAllocator<wchar_t>>;

// template<class T>
// using BulkVector = std::vector<T, BulkStdAllocator<T>>;
// using BulkString = std::basic_string<char, std::char_traits<char>, BulkStdAllocator<char>>;
// using BulkWString = std::basic_string<wchar_t, std::char_traits<wchar_t>, BulkStdAllocator<wchar_t>>;

template<class T>
using TempVector = std::vector<T>;
using TempString = std::string;

template<class T>
using BulkVector = std::vector<T>;
using BulkString = std::string;

}
