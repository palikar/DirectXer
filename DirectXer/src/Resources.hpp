#pragma once 

#include <Types.hpp>
#include <Memory.hpp>
#include <Containers.hpp>

#include <fmt/format.h>

struct Resources
{
	inline static fmt::basic_memory_buffer<char, 512> FormatBuffer{};
	inline static String ResourcesRoot{""};

	static void Init(std::string_view Root)
	{
		ResourcesRoot = Root;
	}

	static const char* ResolveFilePath(std::string_view t_File)
	{
		FormatBuffer.clear();
		fmt::format_to(FormatBuffer, "{}/{}", ResourcesRoot, t_File);
		return FormatBuffer.c_str();
	}

	static const char* ResolveFilePath(const char* t_File)
	{
		FormatBuffer.clear();
		fmt::format_to(FormatBuffer, "{}/{}", ResourcesRoot, t_File);
		return FormatBuffer.c_str();

	}

};
