#pragma once

#include <DirectXMath.h>
#include <d3d11.h>

#include "Types.hpp"
#include "Glm.hpp"

namespace dx = DirectX;

struct PSConstantBuffer
{
    inline static ID3D11Buffer* id{nullptr};
    
	glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
};

struct VSConstantBuffer
{
	inline static ID3D11Buffer* id{nullptr};
    
	glm::mat4 model{};
	glm::mat4 view{};
	glm::mat4 projection{};
	
};

struct SimpleVertex
{
	glm::vec3 pos;
};

void TransformVertex(SimpleVertex& t_Vertex, glm::mat4 t_Mat)
{
	t_Vertex.pos = t_Mat * t_Vertex.pos;	
}
