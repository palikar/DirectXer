#pragma once

#include "Utils.hpp"
#include "Types.hpp"
#include "Config.hpp"

#include <vector>
#include <string>
#include <robin_hood.h>

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
		return (T*)(Current - len);
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
		assert(CurrentScope < MaxTempScopes);
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
	
	void deallocate(T*, size_type){}

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

template<class Key, class Value>
struct SimdFlatMap
{
	using Node = std::pair<uint16, Value>;
	BulkVector<Node> Nodes;

	void reserve(size_t size)
	{
		Nodes.reserve(size);
	}
	
	std::pair<bool, bool> insert(Node node)
	{
		Nodes.push_back(node);
		return { true, true };
	}

	Value& at(uint32 id)
	{
		auto current = Nodes.data();
		
		__m128i value = _mm_set1_epi16(id);
		
		for (size_t i = 0; i < Nodes.size(); i += 8, ++current)
		{
			__m128i keys = _mm_set_epi16(current[7].first, current[6].first, current[5].first, current[4].first,
										 current[3].first, current[2].first, current[1].first, current[0].first);
			__m128i result = _mm_cmpeq_epi16(keys, value);
			unsigned long mask = (unsigned long) _mm_movemask_epi8(result);
			unsigned long index;
			_BitScanForward(&index, mask);

			if(mask)
			{
				return current[index >> 1].second;
			}
		}
		
		assert(false);
		return current->second;
	}

	auto begin()
	{
		return Nodes.begin();
	}

	auto end()
	{
		return Nodes.end();
	}
	
	const auto begin() const
	{
		return Nodes.begin();
	}

	const auto end() const
	{
		return Nodes.end();
	}			
};

template<class Key, class Value>
using GPUResourceMap = SimdFlatMap<Key, Value>;
