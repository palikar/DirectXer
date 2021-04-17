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

const static inline uint16 RectsCount = 1024u / 2u;
const static inline uint16 ImageAtlasSize = 1024u;
const static inline uint8 MaxAtlases = 10u;
const static inline uint16 MaxWidthForPacking = 1024u;
const static inline uint16 MaxHeightForPacking = 1024u;
