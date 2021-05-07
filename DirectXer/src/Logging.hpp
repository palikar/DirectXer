#pragma once

#include <Types.hpp>
#include <Platform.hpp>

#include <fmt/format.h>
#include <fmt/color.h>

#include <cassert>
#include <type_traits>

struct Logger
{
	fmt::basic_memory_buffer<char, Kilobytes(1)> formatBuffer;
	
    template<typename ... Args>
    void PrintLog(const char* t_File, uint32 t_Line, const char* t_Format, Args ... t_Args)
	{
		formatBuffer.clear();
		fmt::format_to(formatBuffer, "[{}:{}] ", strrchr(t_File, '\\') + 1, t_Line);
		fmt::format_to(formatBuffer, t_Format, t_Args ... );
		fmt::format_to(formatBuffer, "\n");
 
		PlatformLayer::SetOuputColor(PlatformLayer::ConsoleForeground::WHITE);
		PlatformLayer::WriteStdOut(formatBuffer.data(), formatBuffer.size());
	}

	template<typename ... Args>
    void PrintError(const char* t_File, uint32 t_Line, const char* t_Format, Args ... t_Args)
	{
		formatBuffer.clear();

		fmt::format_to(formatBuffer, "[{}:{}] ", strrchr(t_File, '\\') + 1, t_Line);
		fmt::format_to(formatBuffer, t_Format, t_Args ... );
		fmt::format_to(formatBuffer, "\n");

		PlatformLayer::SetOuputColor(PlatformLayer::ConsoleForeground::RED);
		PlatformLayer::WriteErrOut(formatBuffer.data(), formatBuffer.size());
	}
	
    template<typename ... Args>
    void Print(const char* t_Format, Args ... t_Args)
	{
		formatBuffer.clear();

		fmt::format_to(formatBuffer, t_Format, t_Args ... );
		
		PlatformLayer::SetOuputColor(PlatformLayer::ConsoleForeground::WHITE);
		PlatformLayer::WriteStdOut(formatBuffer.data(), formatBuffer.size());
	}

};

inline Logger gLogger;

#ifdef _DEBUG

#define DXWARNING(MSG, ...) gLogger.PrintError(__FILE__, __LINE__, MSG, __VA_ARGS__)
#define DXERROR(MSG, ...) gLogger.PrintError(__FILE__, __LINE__, MSG, __VA_ARGS__); int* p = nullptr; *p = 4
#define DXLOG(MSG, ...) gLogger.PrintLog(__FILE__, __LINE__, MSG, __VA_ARGS__)
#define DXDEBUG(MSG, ...) gLogger.PrintLog(__FILE__, __LINE__, MSG, __VA_ARGS__)
#define DXPRINT(MSG, ...) gLogger.Print(MSG, __VA_ARGS__)

#define Assert(VALUE, MSG, ...) do { if (!(VALUE)) { gLogger.PrintError(__FILE__, __LINE__, MSG, __VA_ARGS__); int* p = nullptr; *p = 4; } } while(false)

#else

#define DXWARNING(MSG, ...)
#define DXERROR(MSG, ...)
#define DXLOG(MSG, ...)  gLogger.PrintLog(__FILE__, __LINE__, MSG, __VA_ARGS__)
#define DXDEBUG(MSG, ...)
#define DXPRINT(MSG, ...)

#define Assert(VALUE, MSG, ...)

#endif 



