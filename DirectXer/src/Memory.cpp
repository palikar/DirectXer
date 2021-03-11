#include "Memory.hpp"


const size_t Memory::TempMemoryRequired = Megabytes(256);
const size_t Memory::BulkMemoryRequired = Megabytes(16);
const size_t Memory::TotalMemoryRequired = TempMemoryRequired + BulkMemoryRequired;

MemoryState Memory::g_Memory{0};
MemoryArena Memory::g_TempScope{0};

static const inline size_t SIZE_BYTES = sizeof(size_t);

static bool ArenaHasPlace(MemoryArena t_Arena, size_t t_Size)
{
	return t_Size <= (t_Arena.MaxSize - t_Arena.Size);
}

static void* ArenaAllocation(MemoryArena& t_Arena, size_t t_Size)
{
	auto memory = t_Arena.Get<size_t>(t_Size + SIZE_BYTES);
	*memory = t_Size;
	t_Arena.Size += t_Size + SIZE_BYTES;
	t_Arena.Current += t_Size + SIZE_BYTES;
	
	return memory + 1;
}

static bool EnoughTempMemory(size_t t_Size)
{
	return t_Size <= (Memory::g_Memory.TempMemoryMaxSize - Memory::g_Memory.TempMemorySize);
}

static bool EnoughBulkMemory(size_t t_Size)
{
	return t_Size <= (Memory::g_Memory.BulkMemoryMaxSize - Memory::g_Memory.BulkMemorySize);
}

static size_t BlockSize(void* t_Mem)
{
	return *(size_t*)((char*)t_Mem - SIZE_BYTES);
}

static void SetBlockSize(void* t_Mem, size_t t_Size)
{
	*((size_t*)t_Mem - 1) = t_Size;
}

void Memory::InitMemoryState()
{
	g_Memory.TempMemory = (char*)Platform::Allocate(TotalMemoryRequired);
	g_Memory.TempMemorySize = 0;
	g_Memory.TempMemoryMaxSize = TempMemoryRequired;
	g_Memory.TempMemoryCurrent = Memory::g_Memory.TempMemory;

	g_Memory.BulkMemory = Memory::g_Memory.TempMemory + TempMemoryRequired;
	g_Memory.BulkMemoryCurrent = g_Memory.BulkMemory;
	g_Memory.BulkMemorySize = 0;
	g_Memory.BulkMemoryMaxSize = BulkMemoryRequired;
}

MemoryArena Memory::GetTempArena(size_t t_Size)
{
	assert(EnoughTempMemory(t_Size));

	MemoryArena arena;
	arena.MaxSize = t_Size;
	arena.Size = 0;
	arena.Memory = Memory::g_Memory.TempMemoryCurrent;
	arena.Current = arena.Memory;

	Memory::g_Memory.TempMemoryCurrent += t_Size;
	Memory::g_Memory.TempMemorySize += t_Size;

	return arena;
}

void Memory::DestoryTempArena(MemoryArena&)
{
	
}

void Memory::ResetTempMemory()
{
	Memory::g_Memory.TempMemoryCurrent = Memory::g_Memory.TempMemory;
	Memory::g_Memory.TempMemorySize = 0;
	Memory::g_TempScope = {0};
}

void Memory::EstablishTempScope(size_t t_Size)
{
	g_TempScope = GetTempArena(t_Size);
}

void Memory::ResetTempScope()
{
	g_TempScope.Size = 0;
	g_TempScope.Current = g_TempScope.Memory;
}


void Memory::ReleaseTempScope()
{
	g_TempScope = {0};
}

void* Memory::TempAlloc(size_t t_Size)
{
	assert(Memory::g_TempScope.Memory);

	if(ArenaHasPlace(Memory::g_TempScope, t_Size))
	{
		return ArenaAllocation(Memory::g_TempScope, t_Size);
	}

	assert(false);
	return nullptr;
}

void* Memory::TempRealloc(void* t_Mem, size_t t_Size)
{
	if (!t_Mem) return Memory::TempAlloc(t_Size);

	auto oldSize = BlockSize(t_Mem);
	
	if(((char*)t_Mem - SIZE_BYTES) == (Memory::g_TempScope.Current - oldSize - SIZE_BYTES))
	{
		Memory::g_TempScope.Size += t_Size - oldSize;
		Memory::g_TempScope.Current += t_Size - oldSize;
		SetBlockSize(t_Mem, t_Size);
		return t_Mem;
	}

	auto newBlock = TempAlloc(t_Size);
	memcpy(newBlock, t_Mem, oldSize);
	return newBlock;
}

void Memory::TempDealloc(void*)
{}

void* Memory::BulkGet(size_t t_Size)
{
	assert(EnoughBulkMemory(t_Size));
	auto res = g_Memory.BulkMemoryCurrent;

	g_Memory.BulkMemoryCurrent += t_Size;
	g_Memory.BulkMemorySize += t_Size;
	
	return res;
}
	

void* TempAlloc(size_t size)
{
	return Memory::TempAlloc(size);
}

void* TempRealloc(void* mem, size_t size)
{
	return Memory::TempRealloc(mem, size);
}

void* TempFree(void* mem)
{
	//Note: Do nothing
	return nullptr;
}
