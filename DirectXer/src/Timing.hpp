#pragma once

#include <fmt/format.h>
#include <fmt/color.h>

#include <Platform.hpp>
#include <Memory.hpp>
#include <Types.hpp>
#include <Logging.hpp>
#include <Utils.hpp>

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

}

#define DxTimedBlock(MSG) detail::TimedBlock ANONYMOUS_VARIABLE(__timedBlock)(MSG)
