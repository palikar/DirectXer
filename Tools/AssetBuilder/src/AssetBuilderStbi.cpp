#include <stdlib.h>

//@Note : The sbi setup expects these functions to be difined by when linking
// against the binary

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
