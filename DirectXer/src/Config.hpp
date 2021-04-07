#pragma once

#define USE_CUSTOM_ALLOCATORS 1

#if _DEBUG
static inline const bool DebugBuild = true;
#else
static inline const bool DebugBuild = false;
#endif

// @Most of the time we probably don't need to release resources; enable this
// only to check if there are some memory leaks
static inline const bool CleanDestroy = false;
