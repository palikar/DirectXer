#pragma once

#include <Types.hpp>

enum SystemTag
{
	GPURes_VertexBuffer,
	GPURes_IndexBuffer,
	GPURes_Texture,
	GPURes_ConstantBuffer,

	Phase_Init,
	Phase_Update,
	Phase_Rendering,

	Memory_Bulk,
	Memory_GameState,
	Memory_2DRendering,
	Memory_3DRendering,
	Memory_GPUResource,
	Memory_Audio,

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
	"Rendering",

	"Bulk Memory",
	"GameState Memory",
	"2DRendering Memory",
	"3DRendering Memory",
	"GPUResource Memory",
	"Audio Memory",

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
