#pragma once

#include <Types.hpp>
#include <Glm.hpp>

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
	glm::vec3 cameraPos;
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

struct MtlVertex
{
	glm::vec3 pos;
	glm::vec2 uv;
	glm::vec3 normal;
};

struct MtlInstanceData
{
	glm::mat4 model;
	glm::mat4 invModel;
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
	TT_LINES     = 1,
	TT_TRIANGLE_STRIP
};

enum BlendingState : uint8
{
	BS_AlphaBlending = 0,
	BS_PremultipliedAlpha = 1,

	BS_Count,
};

enum DepthStencilState : uint8
{
	DSS_Normal = 0,
	DSS_2DRendering,

	DSS_Count,
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
	SF_QUAD    = 2,
	SF_MTL     = 3,
	SF_MTLInst = 4,

	SF_COUNT
};

enum ShaderType : uint8
{
    ST_TEX         = 0,
	ST_COLOR       = 1,
	ST_SKY         = 2,
	ST_TEX_SIMPLE  = 3,
	ST_PHONG       = 4,
				   
	ST_RECT        = 5,
	ST_CIRCLE      = 6,
				   
	ST_SIMPLE_QUAD = 7,

	ST_1_MTL       = 8,
	ST_2_MTL       = 9,
	
	ST_COUNT
};

enum ShaderConfiguration
{
	// Debug Rendering
	SC_DEBUG_COLOR       = SF_DEBUG		| (ST_COLOR			<< 8),
	SC_DEBUG_TEX         = SF_DEBUG		| (ST_TEX			<< 8),
	SC_DEBUG_SKY         = SF_DEBUG		| (ST_SKY			<< 8),
	SC_DEBUG_SIMPLE_TEX  = SF_DEBUG		| (ST_TEX_SIMPLE	<< 8),
	SC_DEBUG_PHONG       = SF_DEBUG		| (ST_PHONG			<< 8),

	// 2D rendering
	SC_2D_RECT           = SF_2D		| (ST_RECT			<< 8),
	SC_2D_CIRCLE         = SF_2D		| (ST_CIRCLE		<< 8),
	
	SC_QUAD_SIMPLE       = SF_QUAD		| (ST_SIMPLE_QUAD	<< 8),

	SC_MTL_1             = SF_MTL		| (ST_1_MTL			<< 8),
	SC_MTL_2             = SF_MTL		| (ST_2_MTL			<< 8),

	SC_MTL_2_INSTANCED   = SF_MTLInst	| (ST_2_MTL			<< 8),
	
	SC_COUNT
};

struct TextureDescription
{
	uint16 Width;
	uint16 Height;
	TextureFormat Format;
};

struct RenderTargetDescription
{
	uint16 Width;
	uint16 Height;
	TextureFormat Format;
	bool NeedsDepthStencil;
};

using TextureId = uint16;
using VertexBufferId = uint16;
using IndexBufferId = uint16;
using ConstantBufferId = uint16;

struct RTObject
{
	TextureId Color;
	TextureId DepthStencil;
};

struct SwapChainSettings
{
	void* Display;
	float Width;
	float Height;
};

struct GPUMemoryReport
{
	uint64 Budget;
	uint64 Usage;
};

struct GPUTimingResult
{
	float Time;	
};

struct GPUStatsResult
{
	uint64 VerticesCount;
	uint64 PrimitivesCount;
	uint64 VSInvocationsCount;
	uint64 PSInvocationsCount;
};

inline void TransformVertex(SimpleVertex& t_Vertex, glm::mat4 t_Mat)
{
	t_Vertex.pos = t_Mat * glm::vec4(t_Vertex.pos, 1.0f);
}

inline void TransformVertex(ColorVertex& t_Vertex, glm::mat4 t_Mat)
{
	t_Vertex.pos = t_Mat * glm::vec4(t_Vertex.pos, 1.0f);
}

inline TextureFormat PngFormat(int channels)
{
	if(channels == 1) return TF_A;
	if(channels == 4) return TF_RGBA;

	return TF_UNKNOWN;
}

inline uint8 BytesPerPixel(TextureFormat format)
{
	switch (format)
	{
	  case TF_RGBA: return 4;
	  case TF_A: return 1;
	  case TF_R: return 1;
	}
	
	return 0;
}
