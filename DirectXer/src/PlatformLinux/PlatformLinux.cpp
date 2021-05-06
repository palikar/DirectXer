#include <Memory.hpp>

#include "PlatformLinux.hpp"

void LinuxPlatformLayer::Init()
{
    StdOutHandle = STDOUT_FILENO;
    ErrOutHandle = STDOUT_FILENO;
}


void LinuxPlatformLayer::SetOuputColor(ConsoleForeground color)
{

}

void LinuxPlatformLayer::WriteStdOut(const char* data, size_t len)
{
    write(StdOutHandle, data, len);
}

void LinuxPlatformLayer::WriteErrOut(const char* data, size_t len)
{
    write(ErrOutHandle, data, len);
}

void* LinuxPlatformLayer::Allocate(size_t t_Size)
{
    return (void*)mmap(NULL, t_Size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
}

LinuxPlatformLayer::FileHandle LinuxPlatformLayer::OpenFileForReading(const char* t_Path)
{
    int fd = open(t_Path, O_RDONLY, S_IRUSR | S_IWUSR);

    return fd;
}

size_t LinuxPlatformLayer::FileSize(LinuxPlatformLayer::FileHandle handle)
{
    struct stat statBuf;
    fstat(handle, &statBuf);
    return statBuf.st_size;
}

void LinuxPlatformLayer::ReadFileIntoArena(LinuxPlatformLayer::FileHandle handle, size_t size, MemoryArena& t_Arena)
{
    read(handle, t_Arena.Memory, size);
}

void LinuxPlatformLayer::CloseFile(LinuxPlatformLayer::FileHandle handle)
{

}

bool LinuxPlatformLayer::IsValidPath(const char* data)
{

    return true;
}

uint64 LinuxPlatformLayer::Clock()
{
    struct tms timeInfo;
    times(&timeInfo);
    auto clockTicksPerSecond = sysconf(_SC_CLK_TCK);

    return (uint64)(1000.0f * 1000.0f * 1000.0f * (float)timeInfo.tms_utime / (clockTicksPerSecond));
}


