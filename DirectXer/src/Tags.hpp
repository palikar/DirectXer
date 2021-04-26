#pragma once

enum SystemTag
{
	GPURes_VertexBuffer   = 0,
	GPURes_IndexBuffer    = 1,
	GPURes_Texture        = 2,
	GPURes_ConstantBuffer = 3,

	Tags_Count,
};

static inline const char* gSystemTagNames[] = {
	"VertexBuffer",
	"IndexBuffer",
	"Texture",
	"ConstantBuffer",
};

