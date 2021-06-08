#include <limits>

#include <Types.hpp>
#include <Glm.hpp>
#include <smmintrin.h>
#include <immintrin.h>
#include <xmmintrin.h>
#include <mmintrin.h>

struct alignas(16) AABBNode
{
    int16 MinMaxX[8]; // (minX maxX), (minX maxX), (minX maxX), (minX maxX)
    int16 MinMaxY[8];
    int16 MinMaxZ[8];
    uint64 Index_1_2;
    uint64 Index_3_4;
};

AABBNode BVH[32];


float MAX = 250.0f;
float MIN = -250.0f;
uint16 STEPS = 65535;
float SCALE = ((MAX - MIN) / STEPS);


void SetBoxI(AABBNode& boxRef, float3 min, float3 max, uint8 box)
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

auto Simd_Mad(__m128 a, __m128 x, __m128 b)
{
	return _mm_add_ps(_mm_mul_ps(a, x), b);
}

const __m128 SIMD_INF = _mm_set1_ps(std::numeric_limits<float>::infinity());

int main()
{
	float3 min1{-10.0f, -10.0f, 10.0f};
	float3 max1{10.0f, 10.0f, 15.0f};

	float3 min2{-5.0f, 5.0f, 1.0f};
	float3 max2{-1.0f, 1.0f, 5.0f};

	float3 min3{5.0f, -5.0f, 1.0f};
	float3 max3{1.0f, -1.0f, 5.0f};
	
	float3 min4{-5.0f, -5.0f, 1.0f};
	float3 max4{-1.0f, -1.0f, 5.0f};

	AABBNode box;
	SetBoxI(box, min1, max1, 0);
	SetBoxI(box, min2, max2, 1);
	SetBoxI(box, min3, max3, 2);
	SetBoxI(box, min4, max4, 3);


	// === one time thing ===
	
	float3 rayOrigin{0.0f, 0.0f, 0.0f};
	float3 rayDir{0.0f, 0.0f, 1.0f};
	rayDir = glm::normalize(rayDir);
	
	auto decodeScale = _mm_set1_ps(SCALE);
	auto originX = _mm_set1_ps(0.0f - rayOrigin.x);
	auto originY = _mm_set1_ps(0.0f - rayOrigin.y);
	auto originZ = _mm_set1_ps(0.0f - rayOrigin.z);

	// === decode phase ===

	auto packedMinMaxX = _mm_load_si128((__m128i*)box.MinMaxX);
	auto packedMinX = _mm_srai_epi32(_mm_slli_epi32(_mm_unpacklo_epi16(packedMinMaxX, _mm_setzero_si128()), 16), 16);
	auto packedMaxX = _mm_srai_epi32(_mm_slli_epi32(_mm_unpackhi_epi16(packedMinMaxX, _mm_setzero_si128()), 16), 16);
	auto minX = Simd_Mad(_mm_cvtepi32_ps(packedMinX), decodeScale, originX);
	auto maxX = Simd_Mad(_mm_cvtepi32_ps(packedMaxX), decodeScale, originX);

	auto packedMinMaxY = _mm_load_si128((__m128i*)box.MinMaxY);
	auto packedMinY = _mm_srai_epi32(_mm_slli_epi32(_mm_unpacklo_epi16(packedMinMaxY, _mm_setzero_si128()), 16), 16);
	auto packedMaxY = _mm_srai_epi32(_mm_slli_epi32(_mm_unpackhi_epi16(packedMinMaxY, _mm_setzero_si128()), 16), 16);
	auto minY = Simd_Mad(_mm_cvtepi32_ps(packedMinY), decodeScale, originY);
	auto maxY = Simd_Mad(_mm_cvtepi32_ps(packedMaxY), decodeScale, originY);

	auto packedMinMaxZ = _mm_load_si128((__m128i*)box.MinMaxZ);
	auto packedMinZ = _mm_srai_epi32(_mm_slli_epi32(_mm_unpacklo_epi16(packedMinMaxZ, _mm_setzero_si128()), 16), 16);
	auto packedMaxZ = _mm_srai_epi32(_mm_slli_epi32(_mm_unpackhi_epi16(packedMinMaxZ, _mm_setzero_si128()), 16), 16);
	auto minZ = Simd_Mad(_mm_cvtepi32_ps(packedMinZ), decodeScale, originZ);
	auto maxZ = Simd_Mad(_mm_cvtepi32_ps(packedMaxZ), decodeScale, originZ);

	// === compute phase ===

	const auto dirPickX = _mm_div_ps(_mm_set1_ps(1.0f), _mm_set1_ps(rayDir.x));
	const auto dirPickY = _mm_div_ps(_mm_set1_ps(1.0f), _mm_set1_ps(rayDir.y));
	const auto dirPickZ = _mm_div_ps(_mm_set1_ps(1.0f), _mm_set1_ps(rayDir.z));

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

	const auto t_dcba = _mm_blendv_ps(SIMD_INF, tMaxRaw, maskKept);
	const auto t_ddcb = _mm_shuffle_ps(t_dcba, t_dcba, _MM_SHUFFLE(3, 3, 2, 1));
	const auto t_baab = _mm_shuffle_ps(t_dcba, t_dcba, _MM_SHUFFLE(1, 0, 0, 1));
	const auto dd_cd_bc_ab = _mm_movemask_ps(_mm_cmplt_ps(t_dcba, t_ddcb));
	const auto bd_ad_ac_bb = _mm_movemask_ps(_mm_cmplt_ps(t_baab, t_ddcb));
	const auto bd_ad_ac_cd_bc_ab = bd_ad_ac_bb * 4 + dd_cd_bc_ab;

	const auto index_1 = _mm_popcnt_u32((~bd_ad_ac_cd_bc_ab)		& 0x19);
	const auto index_2 = _mm_popcnt_u32((bd_ad_ac_cd_bc_ab ^ 0x22)	& 0x23);
	const auto index_3 = _mm_popcnt_u32((bd_ad_ac_cd_bc_ab ^ 0x04)	& 0x0E);
	const auto index_4 = _mm_popcnt_u32((bd_ad_ac_cd_bc_ab)			& 0x34);
	
	
	return 0;
}
