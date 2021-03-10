#define STB_IMAGE_IMPLEMENTATION
#define STB_SPRINTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_RECT_PACK_IMPLEMENTATION


extern void* TempAlloc(size_t size);
extern void* TempRealloc(void* mem, size_t size);
extern void* TempFree(void* mem);

#define STBI_MALLOC(sz)           TempAlloc(sz)
#define STBI_REALLOC(p,newsz)     TempRealloc(p,newsz)
#define STBI_FREE(p)              TempFree(p)


#include <stb_image.h>
#include <stb_image_write.h>
#include <stb_image_resize.h>
#include <stb_sprintf.h>
#include <stb_rect_pack.h>
