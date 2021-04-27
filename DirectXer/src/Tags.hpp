#pragma once

#include <Types.hpp>

enum SystemTag : uint32
{
	GPURes_VertexBuffer	  = 0,
	GPURes_IndexBuffer	  = 1,
	GPURes_Texture		  = 2,
	GPURes_ConstantBuffer = 3,

	Phase_Init			  = 4,
	Phase_Update		  = 5,

	Tags_Count,
};

static inline const char* gSystemTagNames[] =
{
	"VertexBuffer",
	"IndexBuffer",
	"Texture",
	"ConstantBuffer",

	"Init",
	"Update",
};

enum CycleCounterTag : uint32
{
	CC_SimdFlatMap_At	= 0,
	CC_Count,
};

static inline const char* gCycleCounterTagNames[] =
{
	"SimdFlatMap_At",
};
