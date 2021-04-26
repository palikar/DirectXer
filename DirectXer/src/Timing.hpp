#pragma once

#include <fmt/format.h>
#include <fmt/color.h>

#include <Platform.hpp>
#include <Memory.hpp>
#include <Types.hpp>
#include <Logging.hpp>
#include <Utils.hpp>
#include <Config.hpp>

#ifdef DX_PROFILE_BUILD
#define USE_OPTICK (1)
#else
#define USE_OPTICK (0)
#endif

#include <optick.h>

namespace detail
{
struct TimedBlock
{
	const char* FormatString;
	uint64 Time;
	
	TimedBlock(const char* formatString)
	{
		FormatString = formatString;
		Time = PlatformLayer::Clock();
	}
	
	~TimedBlock()
	{
		auto elapsedTime = PlatformLayer::Clock() - Time;
		fmt::print(FormatString, elapsedTime/10000);
	}
};

struct CycleCountedBlock
{
	const char* FormatString;
	const char* Tag;
	uint64 Cycles;
	
	CycleCountedBlock(const char* formatString, const char* tag)
	{
		FormatString = formatString;
		Tag = tag;
		Cycles = __rdtsc();
	}
	
	~CycleCountedBlock()
	{
		auto elapsedCycles = __rdtsc() - Cycles;
		if (!Config::EnableCycleCounters) return;
		fmt::print(FormatString, elapsedCycles, Tag);
	}
};

}

#define DxTimedBlock(MSG) detail::TimedBlock ANONYMOUS_VARIABLE(__timedBlock)(MSG)
#define DxCycleBlock(MSG, TAG) detail::CycleCountedBlock ANONYMOUS_VARIABLE(__cycleBlock)(MSG, TAG)
