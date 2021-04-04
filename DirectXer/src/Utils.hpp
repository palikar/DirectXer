#pragma once

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
