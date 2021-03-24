#pragma once




#define USE_CUSTOM_ALLOCATORS 1

#if _DEBUG
static inline const bool DebugBuild = true;
#else
static inline const bool DebugBuild = false;
#endif
