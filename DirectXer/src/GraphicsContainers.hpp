#pragma once

#include <Containers.hpp>
#include <Timing.hpp>

template<class Key, class Value, SystemTag Tag>
struct SimdFlatMap
{
	using Node = std::pair<uint16, Value>;
	BulkVector<Node> Nodes;

	void reserve(size_t size)
	{
		Nodes.reserve(size);
	}
	
	std::pair<bool, bool> insert(Node node)
	{
		Nodes.push_back(node);
		return { true, true };
	}

	Value& at(uint32 id)
	{
		DxCycleBlock(Tag, CC_SimdFlatMap_At);
		auto current = Nodes.data();
		
		__m128i value = _mm_set1_epi16(id);
		
		for (size_t i = 0; i < Nodes.size(); i += 8, current += 8)
		{
			__m128i keys = _mm_set_epi16(current[7].first, current[6].first, current[5].first, current[4].first,
										 current[3].first, current[2].first, current[1].first, current[0].first);
			__m128i result = _mm_cmpeq_epi16(keys, value);
			unsigned long mask = (unsigned long) _mm_movemask_epi8(result);
			unsigned long index;
			_BitScanForward(&index, mask);

			if(mask)
			{
				return current[index >> 1].second;
			}
		}
		
		Assert(false, "Can't find element in has table.");
		return current->second;
	}

	auto begin()
	{
		return Nodes.begin();
	}

	auto end()
	{
		return Nodes.end();
	}
	
	const auto begin() const
	{
		return Nodes.begin();
	}

	const auto end() const
	{
		return Nodes.end();
	}			
};

template<class Key, class Value, SystemTag Tag>
struct SimdFlatMapDense
{
	using Node = std::pair<uint16, Value>;
	BulkVector<uint16> Nodes;
	BulkVector<Value> Values;

	void reserve(size_t size)
	{
		Nodes.reserve(size);
		Values.reserve(size);
	}
	
	std::pair<bool, bool> insert(Node node)
	{
		Nodes.push_back(node.first);
		Values.push_back(node.second);
		return { true, true };
	}

	Value& at(uint32 id)
	{
		DxCycleBlock(Tag, CC_SimdFlatMap_At);
		auto current = Nodes.data();
		
		__m128i value = _mm_set1_epi16(id);
		
		for (size_t i = 0; i < Nodes.size(); i += 8, current += 8)
		{
			__m128i keys = _mm_set_epi16(current[7], current[6], current[5], current[4],
										 current[3], current[2], current[1], current[0]);
			__m128i result = _mm_cmpeq_epi16(keys, value);
			unsigned long mask = (unsigned long) _mm_movemask_epi8(result);
			unsigned long index;
			_BitScanForward(&index, mask);

			if(mask)
			{
				return Values[i + (index >> 1)];
			}
		}
		
		Assert(false, "Can't find element in has table.");
		return Values[0];
	}

	auto begin()
	{
		return Values.begin();
	}

	auto end()
	{
		return Values.end();
	}
	
	const auto begin() const
	{
		return Values.begin();
	}

	const auto end() const
	{
		return Values.end();
	}			
};

template<class Key, class Value, SystemTag Tag>
using GPUResourceMap = SimdFlatMap<Key, Value, Tag>;
