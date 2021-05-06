#pragma once

#if defined(_WIN32)

#include <PlatformWindows/PlatformWindows.hpp>
using PlatformLayer = WindowsPlatformLayer;

#elif defined(__GNUC__)

#include <PlatformLinux/PlatformLinux.hpp>
using PlatformLayer = LinuxPlatformLayer;

#endif




