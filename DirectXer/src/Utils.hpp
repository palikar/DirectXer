#pragma once

#include <Types.hpp>

#include <fmt/format.h>
#include <fmt/color.h>

#define Bytes(num) size_t(num)
#define Kilobytes(num) size_t(num*1024)
#define Megabytes(num) size_t(num*1024*1024)
#define Gigabytes(num) size_t(num*1024*1024*1024)


#define CONCAT_IMPL(s1, s2) s1##s2
#define CONCAT(s1, s2) CONCAT_IMPL(s1, s2)
#define ANONYMOUS_VARIABLE(str) CONCAT(str, __LINE__)


namespace detail
{

template<typename Fun>
struct __scope_exit {
    Fun __fun;

    ~__scope_exit()
    {
        __fun();
    }

};

enum class __scope_guard{};

template<typename Fun>
__scope_exit<Fun>
operator+ (__scope_guard, Fun&& fun)
{
    return __scope_exit<Fun>{fun};
}

}

#define Defer auto ANONYMOUS_VARIABLE(__guard) = detail::__scope_guard() + [&]()

struct TempFormater
{
	fmt::basic_memory_buffer<char, 512> FormatBuffer{};

	template<typename ... T>
	inline const char* Format(const char* formatString, T&& ... args)
	{
		FormatBuffer.clear();
		fmt::format_to(FormatBuffer, formatString, args ...);
		return FormatBuffer.c_str();
	}
};

template<class T, size_t N>
constexpr size_t Size(T (&)[N])
{
	return N;
}

inline uint32 jenkins_hash(const char* begin)
{

    uint32 hash = 0;

    while (*begin)
    {
        hash += *(begin++);
        hash += hash << 10;
        hash ^= hash >> 6;
    }

    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;

    return hash;
}

#ifdef _DEBUG
#define DxDebugCode(STATEMENT) STATEMENT
#else
#define DxDebugCode(STATEMENT)
#endif

#ifdef DX_PROFILE_BUILD
#define DxProfileCode(STATEMENT) STATEMENT
#else
#define DxProfileCode(STATEMENT) STATEMENT
#endif

#if defined(DX_PROFILE_BUILD) || defined(_DEBUG)
#define DxNonReleseCode(STATEMENT) STATEMENT
#else
#define DxNonReleseCode(STATEMENT)
#endif
