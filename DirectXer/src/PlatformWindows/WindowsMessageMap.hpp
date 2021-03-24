#pragma once

#include "IncludeWin.hpp"
#include "Types.hpp"

#include <unordered_map>

class WindowsMessageMap
{
public:
    WindowsMessageMap();

	std::string operator()(DWORD msg, LPARAM lp, WPARAM wp) const;

	std::unordered_map<DWORD, std::string> Map;
};
