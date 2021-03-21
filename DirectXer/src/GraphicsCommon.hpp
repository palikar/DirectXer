#pragma once

#include "Types.hpp"
#include "Glm.hpp"

struct PSConstantBuffer
{
	glm::vec3 cameraPos;
	uint32 shaderType;
};

struct VSConstantBuffer
{
    
	glm::mat4 model{};
	glm::mat4 view{};
	glm::mat4 projection{};
	glm::mat4 invModel;
	uint32 shaderType;
	uint64 _padding;
};

struct SimpleVertex
{
	glm::vec3 pos;
};

struct ColorVertex
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 uv;
	glm::vec3 normal;
};

struct Vertex2D
{
	glm::vec2 pos;
	glm::vec2 uv;
	glm::vec4 color;
	glm::vec3 additional;
	uint32 type;
};

enum TopolgyType : uint8
{
	TT_TRIANGLES = 0,
	TT_LINES     = 1
};

enum BlendingState : uint8
{
	BS_AlphaBlending = 0,
	BS_PremultipliedAlpha = 1,

	BS_Count,
};

enum TextureFormat
{
	TF_RGBA = 0,
	TF_A,
	TF_R,

	TF_UNKNOWN,
};

enum RasterizationState : uint8
{
	RS_NORMAL = 0,
	RS_DEBUG,

	RS_COUNT
};

enum ShaderFile : uint8
{
	SF_DEBUG   = 0,
	SF_2D      = 1,

	SF_COUNT
};

enum ShaderType : uint8
{
    ST_TEX        = 0,
	ST_COLOR      = 1,
	ST_SKY        = 2,
	ST_TEX_SIMPLE = 3,
	ST_PHONG      = 4,

	ST_RECT       = 5,
	ST_CIRCLE     = 6,

	ST_COUNT
};

enum ShaderConfig
{
	// Debug Rendering
	SC_DEBUG_COLOR       = SF_DEBUG | (ST_COLOR      << 8),
	SC_DEBUG_TEX         = SF_DEBUG | (ST_TEX        << 8),
	SC_DEBUG_SKY         = SF_DEBUG | (ST_SKY        << 8),
	SC_DEBUG_SIMPLE_TEX  = SF_DEBUG | (ST_TEX_SIMPLE << 8),
	SC_DEBUG_PHONG       = SF_DEBUG | (ST_PHONG      << 8),

	// 2D rendering
	SC_2D_RECT           = SF_2D    | (ST_RECT      << 8),
	SC_2D_CIRCLE         = SF_2D    | (ST_CIRCLE    << 8),
	
	SC_COUNT
};

inline void TransformVertex(SimpleVertex& t_Vertex, glm::mat4 t_Mat)
{
	t_Vertex.pos = t_Mat * glm::vec4(t_Vertex.pos, 1.0f);
}

inline void TransformVertex(ColorVertex& t_Vertex, glm::mat4 t_Mat)
{
	t_Vertex.pos = t_Mat * glm::vec4(t_Vertex.pos, 1.0f);
}
