#pragma once

#include <Utils.hpp>
#include <Types.hpp>
#include <Config.hpp>
#include <Logging.hpp>
#include <Timing.hpp>
#include <GraphicsCommon.hpp>

#include <vector>
#include <string>
#include <robin_hood.h>

struct MemoryArena
{
	char* Memory;
	char* Current;
	size_t MaxSize;
	size_t Size;

	void Put(const void* data, size_t len);
	void* GetMemory(size_t len);
	void Reset();
	
	template<typename T>
	void Put(const T& value)
	{
		Put(&value, sizeof(T));
	}

	template<typename T = void>
	T* Get(size_t len)
	{
		return (T*)GetMemory(len);
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

struct TempScopesHolder
{
	inline static const uint8 MaxTempScopes = 8;
 
	MemoryArena Scopes[MaxTempScopes];
	uint8 CurrentScope{0};

	inline MemoryArena& GetCurrentArena()
	{
		return Scopes[CurrentScope - 1];
	}
	
	inline void PushScope(MemoryArena t_Arena)
	{
		Assert(CurrentScope < MaxTempScopes, "Too many temporary memory scopes: {}", MaxTempScopes);
		Scopes[CurrentScope++] = t_Arena;
	}

	inline void PopScope()
	{
		Scopes[CurrentScope--] = MemoryArena{0};
	}
};

struct Memory
{
	const static size_t TempMemoryRequired;
	const static size_t BulkMemoryRequired;
	const static size_t TotalMemoryRequired;
	static MemoryState g_Memory;
	static TempScopesHolder g_TempScopes;

	static void* BulkGet(size_t t_Size);

	template<typename T>
	static T* BulkGet(size_t t_Size = 1)
	{
		return (T*)BulkGet(t_Size*sizeof(T));
	}

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
	static void EndTempScope();
	static void* TempAlloc(size_t len);
	static void* TempRealloc(void* mem, size_t len);
	static void TempDealloc(void*);

    //  @Note: Reset the Current temp global temp scope arena
	// to its initial state 
	static void ResetTempScope();

	// @Note: Reset the whole temporary memory
	// in one big go; all arenas should be treated
	// as invalid after that
	static void ResetTempMemory();

	// @Note: Initalize the whole memory by requesting memory from the OS
	static void InitMemoryState();
};

template<typename T>
inline T& ReadBlob(char* &Current)
{
	auto res = (T*)Current;
	Current += sizeof(T);
	return *res;
}

template<typename T>
inline T& ReadBlobAndMove(char* &Current, size_t size)
{
	auto res = (T*)Current;
	Current += size;
	return *res;
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
	
	void deallocate(T*, size_type)
	{
		//Assert(false, "Something using BulkStorage is trying to deallocate. This is not good.");
	}

	inline bool operator==(BulkStdAllocator const&) const { return true; }		
};

#if USE_CUSTOM_ALLOCATORS

template<class T>
using TempVector = std::vector<T, TempStdAllocator<T>>;
using TempString = std::basic_string<char, std::char_traits<char>, TempStdAllocator<char>>;
using TempWString = std::basic_string<wchar_t, std::char_traits<wchar_t>, TempStdAllocator<wchar_t>>;

template<class T>
using BulkVector = std::vector<T, BulkStdAllocator<T>>;
using BulkString = std::basic_string<char, std::char_traits<char>, BulkStdAllocator<char>>;
using BulkWString = std::basic_string<wchar_t, std::char_traits<wchar_t>, BulkStdAllocator<wchar_t>>;

using String = std::string_view;

template<class Key, class Value>
using Map = robin_hood::unordered_map<Key, Value>;

#else

template<class T>
using TempVector = std::vector<T>;
using TempString = std::string;

template<class T>
using BulkVector = std::vector<T>;
using BulkString = std::string;

using String = std::string_view;

#endif
