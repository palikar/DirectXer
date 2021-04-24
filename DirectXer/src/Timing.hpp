#pragma once

#include <fmt/format.h>
#include <fmt/color.h>

#include <Platform.hpp>
#include <Memory.hpp>
#include <Types.hpp>
#include <Logging.hpp>



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
