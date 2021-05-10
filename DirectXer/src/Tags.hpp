#pragma once

#include <Types.hpp>

enum SystemTag
{
	GPURes_VertexBuffer	  = 0,
	GPURes_IndexBuffer	  = 1,
	GPURes_Texture		  = 2,
	GPURes_ConstantBuffer = 3,

	Phase_Init			  = 4,
	Phase_Update		  = 5,

	Memory_Bulk  		  = 6,
	Memory_GameState  	  = 7,


	Tag_Unknown,
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

	"Bulk Memory",
	"GameState Memory",

	"Unknow",
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
