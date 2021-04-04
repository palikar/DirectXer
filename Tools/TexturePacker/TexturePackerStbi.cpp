#include <stdlib.h>

void* TempAlloc(size_t size)
{
	return malloc(size);
}

void* TempRealloc(void* mem, size_t size)
{
	return realloc(mem, size);
}

void* TempFree(void* mem)
{
	free(mem);
	return nullptr;
}
