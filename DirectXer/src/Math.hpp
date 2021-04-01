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

struct Rectangle2D
{
	glm::vec2 Position;
	glm::vec2 Size;

};

inline bool IntersectRects(const Rectangle2D& rect1, const Rectangle2D& rect2)
{
	auto max1 = rect1.Position + rect1.Size;
	auto max2 = rect2.Position + rect2.Size;

	auto min1 = rect1.Position;
	auto min2 = rect2.Position;

	return !(max1.x < min2.x || min1.x > max2.x || max1.y < min2.y || min1.y > max2.y);
}
