#pragma once

#include "Types.h"
#include "IncludeWin.h"

#include <fmt/format.h>
#include <fmt/color.h>


struct Logger
{

	enum class ConsoleForeground : DWORD
	{
		BLACK = 0,
		DARKBLUE = FOREGROUND_BLUE,
		DARKGREEN = FOREGROUND_GREEN,
		DARKCYAN = FOREGROUND_GREEN | FOREGROUND_BLUE,
		DARKRED = FOREGROUND_RED,
		DARKMAGENTA = FOREGROUND_RED | FOREGROUND_BLUE,
		DARKYELLOW = FOREGROUND_RED | FOREGROUND_GREEN,
		DARKGRAY = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
		GRAY = FOREGROUND_INTENSITY,
		BLUE = FOREGROUND_INTENSITY | FOREGROUND_BLUE,
		GREEN = FOREGROUND_INTENSITY | FOREGROUND_GREEN,
		CYAN = FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE,
		RED = FOREGROUND_INTENSITY | FOREGROUND_RED,
		MAGENTA = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE,
		YELLOW = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN,
		WHITE = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
	};

	// @Imporve : Get everthing needed upfront
	// @Imporve : Buffer the thing a little bit
	// @Imporve : Add logging severities support
	
    template<typename ... Args>
    void PrintLog(const char* t_File, uint32 t_Line, const char* t_Format, Args ... t_Args)
	{

		auto hOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hOutHandle, (DWORD)ConsoleForeground::WHITE);
		fmt::basic_memory_buffer<char, 512> out;
		fmt::format_to(out, "[{}:{}] ", strrchr(t_File, '\\') + 1, t_Line);
		fmt::format_to(out, t_Format, t_Args ... );
		fmt::format_to(out, "\n");
		
		WriteFile(hOutHandle, out.data(), out.size(), NULL, NULL);
	}

	template<typename ... Args>
    void PrintError(const char* t_File, uint32 t_Line, const char* t_Format, Args ... t_Args)
	{

		auto hOutHandle = GetStdHandle(STD_ERROR_HANDLE );
		SetConsoleTextAttribute(hOutHandle, (DWORD)ConsoleForeground::RED);
		
		fmt::basic_memory_buffer<char, 512> out;
		fmt::format_to(out, "[{}:{}] ", strrchr(t_File, '\\') + 1, t_Line);
		fmt::format_to(out, t_Format, t_Args ... );
		fmt::format_to(out, "\n");
		
		WriteFile(hOutHandle, out.data(), out.size(), NULL, NULL);
	}
	
    template<typename ... Args>
    void Print(const char* t_Format, Args ... t_Args)
	{

		auto hOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hOutHandle, (DWORD)ConsoleForeground::WHITE);
		fmt::basic_memory_buffer<char, 512> out;
		fmt::format_to(out, t_Format, t_Args ... );
		fmt::format_to(out, "\n");
		WriteFile(hOutHandle, out.data(), out.size(), NULL, NULL);
	}

	
    
};

inline Logger gLogger;

#ifdef _DEBUG

#define DXERROR(MSG, ...) gLogger.PrintError(__FILE__, __LINE__, MSG, __VA_ARGS__)
#define DXLOG(MSG, ...) gLogger.PrintLog(__FILE__, __LINE__, MSG, __VA_ARGS__)
#define DXPRINT(MSG, ...) gLogger.Print(MSG, __VA_ARGS__)

#else

#define DXERROR(MSG, ...) (void)0
#define DXLOG(MSG, ...) (void)0
#define PRINT(MSG, ...) (void)0

#endif 



