#prgma once

#include <Types.hpp>
#include <Utils.hpp>

struct LinuxPlatformLayer
{
    using FileHandle = int;
	using WindowType = int;

	inline static FileHandle StdOutHandle;
	inline static FileHandle ErrOutHandle;

	enum class ConsoleForeground : DWORD
	{
		BLACK = 0,
		DARKBLUE,
		DARKGREEN,
		DARKCYAN,
		DARKRED,
		DARKMAGENTA,
		DARKYELLOW,
		DARKGRAY,
		GRAY,
		BLUE,
		GREEN,
		CYAN,
		RED,
		MAGENTA,
		YELLOW,
		WHITE,
	};

	static void Init();
	static void SetOuputColor(ConsoleForeground color);
	static void WriteStdOut(const char* data, size_t len);
	static void WriteErrOut(const char* data, size_t len);
	static void* Allocate(size_t t_Size);
	static FileHandle OpenFileForReading(const char* t_Path);
	static size_t FileSize(FileHandle handle);
	static void ReadFileIntoArena(FileHandle handle, size_t size, MemoryArena& t_Arena);
	static void CloseFile(FileHandle handle);
	static bool IsValidPath(const char* data);
	static uint64 Clock();
};
