#pragma once

#include <Glm.hpp>
#include <GraphicsCommon.hpp>

#include <cmath>
#include <utility>
#include <algorithm>

inline constexpr float PI = 3.14159265358979323846f;

// @Todo: Implement fast version of the function below that uses some
// SIMD optimizations; also, we'll probably will have to work on
// concrete types or at least not delegate the transformations on
// another function; we have to keep the context here if we want to
// operate on groups of vertecies and not only one

// @Note: Transfrom some vertecies with a given transform matrix; this
// can potentially be epxesinve operation but it is meant to be used
// only on startup or to precompute something
template<typename T>
static void TransformVertices(glm::mat4 t_Mat, T* t_Vertices, size_t t_Count)
{
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
