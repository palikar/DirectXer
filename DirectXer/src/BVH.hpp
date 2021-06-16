
#include <Types.hpp>
#include <Glm.hpp>
#include <Memory.hpp>
#include <Containers.hpp>


struct AABB
{
	float3 Min;
	float3 Max;
	float _padding;
};

struct BVHNode
{
	AABB box;

	int parentIndex;
	int child1;
	int child2;
	// int objectIndex;
	
};

struct SlowBVH
{
	BulkVector<BVHNode> Nodes;
	int RootIndex;
};

struct Node
{
	int index;
	float inherited;
};

void InsertAABB(SlowBVH& bvh, AABB aabb);

struct alignas(16) AABBNode
{
    int16 MinMaxX[8]; // (minX maxX), (minX maxX), (minX maxX), (minX maxX)
    int16 MinMaxY[8];
    int16 MinMaxZ[8];
    uint32 Data[4];
};

struct FlatAABBs
{
	AABBNode BVH[64];
	float3 Origin;
	uint16 CurrentBox;
	uint16 CurrentNode;

	static inline const float MAX = 250.0f;
	static inline const float MIN = -250.0f;
	static inline const uint16 STEPS = 65535;
	static inline const float SCALE = ((MAX - MIN) / STEPS);

	void ModifyBox(int code, float3 min, float3 max);

	int AddBox(float3 min, float3 max, uint32 data);

	uint32 Cast(float3 rayOrigin, float3 rayDir);
	
};
