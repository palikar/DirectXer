#pragma once

#include <Containers.hpp>
#include <Timing.hpp>

template<class Key, class Value, SystemTag Tag>
struct SimdFlatMap
{
	using Node = std::pair<uint16, Value>;
	BulkVector<Node, Memory_GPUResource> Nodes;

	void reserve(size_t size)
	{
		Nodes.reserve(size);
	}
	
	std::pair<bool, bool> insert(Node node)
	{
		Nodes.push_back(node);
		return { true, true };
	}

	auto value_at(uint32 id)
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
				return Nodes.begin() + i + (index >> 1);
			}
		}
		
		return Nodes.end();
	}

	Value& at(uint32 id)
	{
		auto it = value_at(id);
		Assert(it != Nodes.end(), "Can't find element in has table.");
		return it->second;
	}

	Value erase_at(uint32 id)
	{
		auto it = value_at(id);
		Value res = it->second;
		Nodes.erase(it);
		return res;
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
using GPUResourceMap = SimdFlatMap<Key, Value, Tag>;
