#include <BVH.hpp>

static AABB Union(const AABB a, const AABB b)
{
	AABB result;

	// auto aMax = glm::max(a.Max, a.Min);
	// auto aMin = glm::min(a.Max, a.Min);
	// auto bMax = glm::max(b.Max, b.Min);
	// auto bMin = glm::min(b.Max, b.Min);
	
	// result.Max = glm::max(aMax, bMax);
	// result.Min = glm::min(aMin, bMin);

	const auto aMinMM = _mm_loadu_ps(&a.Min.x);
	const auto aMaxMM = _mm_loadu_ps(&a.Max.x);
	const auto aMin = _mm_min_ps(aMinMM, aMaxMM);
	const auto aMax = _mm_max_ps(aMinMM, aMaxMM);

	const auto bMinMM = _mm_loadu_ps(&b.Min.x);
	const auto bMaxMM = _mm_loadu_ps(&b.Max.x);
	const auto bMin = _mm_min_ps(bMinMM, bMaxMM);
	const auto bMax = _mm_max_ps(bMinMM, bMaxMM);
		
	_mm_storeu_ps(&result.Min.x, _mm_min_ps(aMin, bMin));
	_mm_storeu_ps(&result.Max.x, _mm_max_ps(aMax, bMax));

	return result;
}

static float Area(const AABB& a)
{
	const auto d = _mm_sub_ps(_mm_loadu_ps(&a.Min.x), _mm_loadu_ps(&a.Max.x));
	const auto dd = _mm_shuffle_ps(d, d, _MM_SHUFFLE(3, 0, 2, 1));
	const auto dot = _mm_dp_ps(d, dd, 0X71);
	return 2.0f * (dot.m128_f32[0]);
}

void InsertAABB(SlowBVH& bvh, AABB aabb)
{
	
	const auto newNode = (int)bvh.Nodes.size();
	bvh.Nodes.push_back({ aabb });
	bvh.Nodes.back().child1 = -1;
	bvh.Nodes.back().child2 = -1;
	
	auto& nodes = bvh.Nodes;

	if (newNode == 0)
	{
		bvh.Nodes.back().parentIndex = -1;
		bvh.RootIndex = 0;
		return;
	}

	Memory::EstablishTempScope(Megabytes(3));
	Defer{
		Memory::EndTempScope();
	};


	int best = bvh.RootIndex;
	float bestCost = Area(Union(aabb, nodes[best].box));

	float delta = Area(aabb);

	TempVector<Node> stack;
	stack.reserve(2048);
	stack.push_back({bvh.RootIndex, bestCost - Area(nodes[best].box)});
	while (!stack.empty())
	{
		const float inheritedCost = stack.back().inherited;
		const auto i = (int)stack.back().index;
		
		stack.pop_back();

		auto& node = nodes[i];
		const float totalCost = inheritedCost + Area(Union(node.box, aabb));
		if (totalCost < bestCost )
		{
			bestCost = totalCost;
			best = i;
		}

		const float low = totalCost - Area(node.box);
		if (low + delta < bestCost)
		{
			if (node.child1 != -1) stack.push_back({node.child1, low});
			if(node.child1 != -1) stack.push_back({node.child2, low});
		}

	}

	auto oldParent = nodes[best].parentIndex;
	auto newParent = (int) nodes.size();
	nodes.push_back({});
	nodes[newParent].parentIndex = oldParent;
	nodes[newParent].box = Union(aabb, nodes[best].box);

	if (oldParent == -1)
	{
		nodes[newParent].child1 = best;
		nodes[newParent].child2 = newNode;
		nodes[best].parentIndex = newParent;
		nodes[newNode].parentIndex = newParent;
		bvh.RootIndex = newParent;

	}
	else
	{
		if (nodes[oldParent].child1 == best)
		{
			nodes[oldParent].child1 = newParent;
		}
		else
		{
			nodes[oldParent].child2 = newParent;
		}

		nodes[newParent].child1 = best;
		nodes[newParent].child2 = newNode;
		nodes[best].parentIndex = newParent;
		nodes[newNode].parentIndex = newParent;
	}

	int index = nodes[newNode].parentIndex;
	while (index != -1)
	{
		int child1 = nodes[index].child1;
		int child2 = nodes[index].child2;

		nodes[index].box = Union(nodes[child1].box, nodes[child2].box);
		index = nodes[index].parentIndex;
	}
}


static const __m128 SIMD_INF = _mm_set1_ps(std::numeric_limits<float>::infinity());

static void SetBoxI(AABBNode& boxRef, float3 min, float3 max, uint8 box, float SCALE)
{
	glm::ivec3 minAdj = glm::round(min / SCALE);
	glm::ivec3 maxAdj = glm::round(max / SCALE);

	boxRef.MinMaxX[box + 0] = (int16)minAdj.x;
	boxRef.MinMaxX[box + 4] = (int16)maxAdj.x;

	boxRef.MinMaxY[box + 0] = (int16)minAdj.y;
	boxRef.MinMaxY[box + 4] = (int16)maxAdj.y;

	boxRef.MinMaxZ[box + 0] = (int16)minAdj.z;
	boxRef.MinMaxZ[box + 4] = (int16)maxAdj.z;
}

inline auto Simd_Mad(__m128 a, __m128 x, __m128 b)
{
	return _mm_add_ps(_mm_mul_ps(a, x), b);
}

void FlatAABBs::ModifyBox(int code, float3 min, float3 max)
{
	uint16 node = code >> 16;
	uint16 offset = code & 0xFF;

	auto& aabb = BVH[node];
		
	SetBoxI(aabb, min, max, (uint8)offset, SCALE);
}

int FlatAABBs::AddBox(float3 min, float3 max, uint32 data)
{
	auto& aabb = BVH[CurrentNode];
	aabb.Data[CurrentBox] = data;
		
	SetBoxI(aabb, min, max, (uint8)CurrentBox, SCALE);

	auto result = CurrentBox << 16 | CurrentBox;
		
	++CurrentBox;
	CurrentNode += (CurrentBox & 4) >> 2;
	CurrentBox = CurrentBox & 3;
		
	return result;
}

uint32 FlatAABBs::Cast(float3 rayOrigin, float3 rayDir)
{
	rayDir = glm::normalize(rayDir);
	
	auto decodeScale = _mm_set1_ps(SCALE);
	auto originX = _mm_set1_ps(0.0f - rayOrigin.x);
	auto originY = _mm_set1_ps(0.0f - rayOrigin.y);
	auto originZ = _mm_set1_ps(0.0f - rayOrigin.z);

	const auto dirPickX = _mm_div_ps(_mm_set1_ps(1.0f), _mm_set1_ps(rayDir.x));
	const auto dirPickY = _mm_div_ps(_mm_set1_ps(1.0f), _mm_set1_ps(rayDir.y));
	const auto dirPickZ = _mm_div_ps(_mm_set1_ps(1.0f), _mm_set1_ps(rayDir.z));

	for (size_t i = 0; i < 64; ++i)
	{
		auto box = BVH[i];
		if (box.Data[3] == 0) return 0;

		const auto packedMinMaxX = _mm_load_si128((__m128i*)box.MinMaxX);
		const auto packedMinX = _mm_srai_epi32(_mm_slli_epi32(_mm_unpacklo_epi16(packedMinMaxX, _mm_setzero_si128()), 16), 16);
		const auto packedMaxX = _mm_srai_epi32(_mm_slli_epi32(_mm_unpackhi_epi16(packedMinMaxX, _mm_setzero_si128()), 16), 16);
		const auto minX = Simd_Mad(_mm_cvtepi32_ps(packedMinX), decodeScale, originX);
		const auto maxX = Simd_Mad(_mm_cvtepi32_ps(packedMaxX), decodeScale, originX);

		const auto packedMinMaxY = _mm_load_si128((__m128i*)box.MinMaxY);
		const auto packedMinY = _mm_srai_epi32(_mm_slli_epi32(_mm_unpacklo_epi16(packedMinMaxY, _mm_setzero_si128()), 16), 16);
		const auto packedMaxY = _mm_srai_epi32(_mm_slli_epi32(_mm_unpackhi_epi16(packedMinMaxY, _mm_setzero_si128()), 16), 16);
		const auto minY = Simd_Mad(_mm_cvtepi32_ps(packedMinY), decodeScale, originY);
		const auto maxY = Simd_Mad(_mm_cvtepi32_ps(packedMaxY), decodeScale, originY);

		const auto packedMinMaxZ = _mm_load_si128((__m128i*)box.MinMaxZ);
		const auto packedMinZ = _mm_srai_epi32(_mm_slli_epi32(_mm_unpacklo_epi16(packedMinMaxZ, _mm_setzero_si128()), 16), 16);
		const auto packedMaxZ = _mm_srai_epi32(_mm_slli_epi32(_mm_unpackhi_epi16(packedMinMaxZ, _mm_setzero_si128()), 16), 16);
		const auto minZ = Simd_Mad(_mm_cvtepi32_ps(packedMinZ), decodeScale, originZ);
		const auto maxZ = Simd_Mad(_mm_cvtepi32_ps(packedMaxZ), decodeScale, originZ);

		const auto termMinX = _mm_mul_ps(minX, dirPickX);
		const auto termMaxX = _mm_mul_ps(maxX, dirPickX);
		const auto termMinY = _mm_mul_ps(minY, dirPickY);
		const auto termMaxY = _mm_mul_ps(maxY, dirPickY);
		const auto termMinZ = _mm_mul_ps(minZ, dirPickZ);
		const auto termMaxZ = _mm_mul_ps(maxZ, dirPickZ);

		const auto tMinX = _mm_min_ps(termMaxX, termMinX);
		const auto tMaxX = _mm_max_ps(termMinX, termMaxX);
	
		const auto tMinY = _mm_min_ps(termMaxY, termMinY);
		const auto tMaxY = _mm_max_ps(termMinY, termMaxY);

		const auto tMinZ = _mm_min_ps(termMaxZ, termMinZ);
		const auto tMaxZ = _mm_max_ps(termMinZ, termMaxZ);

		const auto tMin = _mm_max_ps(tMinZ, _mm_max_ps(tMinY, _mm_max_ps(tMinX, _mm_setzero_ps())));

		const auto tMaxRaw = _mm_min_ps(tMaxZ, _mm_min_ps(tMaxY, _mm_min_ps(tMaxX, SIMD_INF)));
		const auto tMax = _mm_min_ps(tMaxRaw, _mm_set1_ps(250.0f));

		const auto maskKept = _mm_cmplt_ps(tMin, tMax);
		const auto keepFlags = _mm_movemask_ps(maskKept);

		const auto numNodes = _mm_popcnt_u32(keepFlags);
		if (numNodes == 1)
		{
			const auto onlyNodeIndex = (keepFlags >> 1) - (keepFlags >> 3);
			return box.Data[onlyNodeIndex];
		}

		const auto t_dcba = _mm_blendv_ps(SIMD_INF, tMaxRaw, maskKept);
		const auto t_ddcb = _mm_shuffle_ps(t_dcba, t_dcba, _MM_SHUFFLE(3, 3, 2, 1));
		const auto t_baab = _mm_shuffle_ps(t_dcba, t_dcba, _MM_SHUFFLE(1, 0, 0, 1));
		const auto dd_cd_bc_ab = _mm_movemask_ps(_mm_cmplt_ps(t_dcba, t_ddcb));
		const auto bd_ad_ac_bb = _mm_movemask_ps(_mm_cmplt_ps(t_baab, t_ddcb));

		const auto bd_ad_ac_cd_bc_ab = bd_ad_ac_bb * 4 + dd_cd_bc_ab;
		const auto index_1 = _mm_popcnt_u32((~bd_ad_ac_cd_bc_ab)		& 0x19);
		const auto index_2 = _mm_popcnt_u32(( bd_ad_ac_cd_bc_ab ^ 0x22)	& 0x23);
		const auto index_3 = _mm_popcnt_u32(( bd_ad_ac_cd_bc_ab ^ 0x04)	& 0x0E);
		const auto index_4 = _mm_popcnt_u32(( bd_ad_ac_cd_bc_ab)		& 0x34);

		if (keepFlags > 0)
		{
			uint32 datas[4];
			datas[index_1] = box.Data[0];
			datas[index_2] = box.Data[1];
			datas[index_3] = box.Data[2];
			datas[index_4] = box.Data[3];
			return datas[0];
		}
	}
		
	return 0;
}
