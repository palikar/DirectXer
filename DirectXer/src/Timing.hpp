#pragma once

#include <fmt/format.h>
#include <fmt/color.h>

#include <Types.hpp>
#include <Utils.hpp>
#include <Config.hpp>
#include <Tags.hpp>
#include <Memory.hpp>
#include <GraphicsCommon.hpp>
#include <Containers.hpp>

#include <optick.h>

struct Telemetry
{
	struct CycleCountedEntry
	{
		uint64 AvgCycles{ 0 };
		uint64 EntriesCount{ 0 };
	};

	struct TimedBlockEntry
	{
		const char* Msg;
		uint64 Time;
	};

	struct MemoryState
	{
		uint64 CurrentMemory;
	};

	static inline Map<uint64, CycleCountedEntry> CycleCounters{};
	static inline Map<uint64, TimedBlockEntry> BlockTimers{};
	static inline MemoryState MemoryStates[Tags_Count]{0};

	static void NewCycleCounterEntry(SystemTag sysTag, CycleCounterTag counterTag, uint64 cycles)
	{
		auto& entry = CycleCounters[(uint64)sysTag << 32 | (uint64)counterTag];
		entry.EntriesCount += 1;
		entry.AvgCycles += cycles;
		entry.AvgCycles /= entry.EntriesCount > 1 ? 2 : 1;
	}

	static void NewTimedBlockEntry(SystemTag sysTag, const char* msg, uint64 time)
	{
		BlockTimers[(uint64)jenkins_hash(msg) << 32 | sysTag] = TimedBlockEntry{msg, time};
	}


	static void AddMemory(SystemTag sysTag, uint64 memory)
	{
		MemoryStates[sysTag].CurrentMemory += memory;
	}

	static void RemoveMemory(SystemTag sysTag, uint64 memory)
	{
		MemoryStates[sysTag].CurrentMemory -= memory;
	}

	static void Init()
	{
		CycleCounters.reserve(32);
		BlockTimers.reserve(32);
	}
};

namespace detail
{

struct TimedBlock
{
	SystemTag SysTag;
	const char* Msg;
	uint64 Time;
	
	TimedBlock(SystemTag sysTag, const char* msg)
	{
		SysTag = sysTag;
		Msg = msg;
		Time = PlatformLayer::Clock();
	}
	
	~TimedBlock()
	{
		auto elapsedTime = PlatformLayer::Clock() - Time;
		Telemetry::NewTimedBlockEntry(SysTag, Msg, elapsedTime/10000);
	}
};

struct CycleCountedBlock
{
	SystemTag SysTag;
	CycleCounterTag CounterTag;
	uint64 Cycles;
	
	CycleCountedBlock(SystemTag sysTag, CycleCounterTag counterTag)
	{
		SysTag = sysTag;
		CounterTag = counterTag;
		Cycles = __rdtsc();
	}
	
	~CycleCountedBlock()
	{
		auto elapsedCycles = __rdtsc() - Cycles;
		if (!Config::EnableCycleCounters) return;
		Telemetry::NewCycleCounterEntry(SysTag, CounterTag, elapsedCycles);
	}	
};

}

#define DxTimedBlock(SYS, MSG) detail::TimedBlock ANONYMOUS_VARIABLE(__timedBlock)(SYS, MSG)
#define DxCycleBlock(SYS, COUNTER) detail::CycleCountedBlock ANONYMOUS_VARIABLE(__cycleBlock)(SYS, COUNTER)

