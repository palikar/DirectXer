#pragma once

#include "Types.h"
#include "IncludeWin.h"

#include <fmt/format.h>


struct Logger
{


    template<typename ... Args>
    void PrintLog(const char* t_File, uint32 t_Line, const char* t_Format, Args ... t_Args)
	{

		auto hOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		fmt::basic_memory_buffer<char, 512> out;
		fmt::format_to(out, "[{}:{}] ", strrchr(t_File, '\\') + 1, t_Line);
		fmt::format_to(out, t_Format, t_Args ...);

		DWORD written{ 0 };
		WriteFile(hOutHandle, out.data(), out.size(), &written, NULL);
	}
    
};

inline Logger gLogger;


#define DXERROR
#define DXLOG(MSG, ...) gLogger.PrintLog(__FILE__, __LINE__, MSG, __VA_ARGS__)
#define PRINT



