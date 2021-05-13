#pragma once

#include <Utils.hpp>
#include <Types.hpp>
#include <Config.hpp>
#include <Logging.hpp>
#include <GraphicsCommon.hpp>
#include <Tags.hpp>

#include <vector>
#include <string>
#include <robin_hood.h>

#if USE_CUSTOM_ALLOCATORS

template<SystemTag Tag = Tag_Unknown>
struct RobinAllocator
{
	static  void* malloc(size_t size)
	{
		return Memory::BulkGet(size, Tag);
	}

	static void free(void* ptr)
	{
	}
};

template<class T>
using TempVector = std::vector<T, TempStdAllocator<T>>;
using TempString = std::basic_string<char, std::char_traits<char>, TempStdAllocator<char>>;
using TempWString = std::basic_string<wchar_t, std::char_traits<wchar_t>, TempStdAllocator<wchar_t>>;

template<class T, SystemTag Tag = Tag_Unknown>
using BulkVector = std::vector<T, BulkStdAllocator<T, Tag>>;
using BulkString = std::basic_string<char, std::char_traits<char>, BulkStdAllocator<char>>;
using BulkWString = std::basic_string<wchar_t, std::char_traits<wchar_t>, BulkStdAllocator<wchar_t>>;

using String = std::string_view;

template<class Key, class Value, SystemTag Tag = Tag_Unknown>
using Map = robin_hood::unordered_map<Key, Value, RobinAllocator<Tag>>;

#else

template<class T>
using TempVector = std::vector<T>;
using TempString = std::string;

template<class T>
using BulkVector = std::vector<T>;
using BulkString = std::string;

using String = std::string_view;

template<class Key, class Value>
using Map = robin_hood::unordered_map<Key, Value>;

#endif


