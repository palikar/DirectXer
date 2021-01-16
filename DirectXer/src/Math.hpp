#pragma once

#include "Glm.hpp"
#include "GraphicsCommon.hpp"


template<typename T>
static void TransformVertices(glm::mat4 t_Mat, T* t_Vertices, size_t t_Count)
{
	size_t index = 0;
	for (size_t i = 0; i < t_Count; ++i)
	{
		TransformVertex(*t_Vertices++, t_Mat);
	}
}
