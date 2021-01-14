#pragma once

#include "Glm.hpp"


static void TransformVertices(glm::mat4 t_Mat, float* t_Vertices, size_t t_Count)
{
	size_t index = 0;
	for (size_t i = 0; i < 3 * t_Count; i+=3)
	{
		auto next = t_Vertices + i;
		auto res = t_Mat * glm::vec4{*(next + 0), *(next + 1), *(next + 2), 1};
		*(next + 0) = res.x;
		*(next + 1) = res.y; 
		*(next + 2) = res.z;		
	}
}
