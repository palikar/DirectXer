#pragma once


#include "IncludeWin.h"
#include <unordered_map>

class WindowsMessageMap
{
public:
    WindowsMessageMap();
    std::string operator()( DWORD msg,LPARAM lp,WPARAM wp ) const;
private:
    std::unordered_map<DWORD,std::string> m_Map;
};
