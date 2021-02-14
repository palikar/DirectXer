#include "Glm.hpp"
#include "Math.hpp"
#include "Geometry.hpp"

static CylinderGeometry cylinderShape{0.005f, 0.005f, 0.25f, 10.0f, 1.0f};
static SphereGeometry lightSphere{0.1f, 4.0f, 2.0f};

static GeometryInfo AxisHelperInfo()
{
	// @Todo: This is a number that is know all the time!
	auto cylinderInfo = CylinderGeometryInfo(cylinderShape);
	
	return {cylinderInfo.vertexCount * 3, cylinderInfo.indexCount * 3, GT_AXISHELPER};
}

static int AxisHelperData(ColorVertex* t_Vertices, std::vector<uint32>& t_Indices)
{
	// @Todo: We _know_ the numbers that are in here
	auto cylinderInfo = CylinderGeometryInfo(cylinderShape);

	CylinderGeometryData(cylinderShape, t_Vertices, t_Indices, 0);
	CylinderGeometryData(cylinderShape, &t_Vertices[cylinderInfo.vertexCount * 1u], t_Indices, cylinderInfo.vertexCount * 1u);
	CylinderGeometryData(cylinderShape, &t_Vertices[cylinderInfo.vertexCount * 2u], t_Indices, cylinderInfo.vertexCount * 2u);

	glm::mat4 transform(1);
	transform = glm::translate(transform, glm::vec3(0.0f, 0.25f * 0.5f, 0.0f));
	TransformVertices(transform, t_Vertices, cylinderInfo.vertexCount);
	
	transform = glm::mat4(1);
	transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(1, 0, 0));
	transform = glm::translate(transform, glm::vec3(0.0f, 0.25f * 0.5f, 0.0f));
	TransformVertices(transform, &t_Vertices[cylinderInfo.vertexCount*1], cylinderInfo.vertexCount);

	transform = glm::mat4(1);
	transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(0, 0, 1));
	transform = glm::translate(transform, glm::vec3(0.0f, -0.25f * 0.5f, 0.0f));
	TransformVertices(transform, &t_Vertices[cylinderInfo.vertexCount*2], cylinderInfo.vertexCount);
	
	for (size_t i = 0; i < cylinderInfo.vertexCount; ++i)
	{
		t_Vertices[i].color = glm::vec3{0.0f, 1.0f, 0.0f};
	}

	for (size_t i = cylinderInfo.vertexCount; i < cylinderInfo.vertexCount*2; ++i)
	{
		t_Vertices[i].color = glm::vec3{0.0f, 0.0f, 1.0f};
	}

	for (size_t i = cylinderInfo.vertexCount*2; i < cylinderInfo.vertexCount*3; ++i)
	{
		t_Vertices[i].color = glm::vec3{1.0f, 0.0f, 0.0f};
	}
	
	
	return 0;
}

struct CameraHelper
{
	glm::mat4 projection;
};

static GeometryInfo CameraHelperInfo(CameraHelper t_Camera)
{
	return {21 + 1, 25 * 2 * 2, GT_CAMHELPER};
}

static int CameraHelperData(CameraHelper t_Camera, ColorVertex* t_Vertices, std::vector<uint32>& t_Indices)
{
	const glm::mat4 invProj = glm::inverse(t_Camera.projection);
	const float w = 1.0f, h = 1.0f;
	const glm::vec3 cyan{1.0f, 0.0f, 1.0f};
	const glm::vec3 red{1.0f, 0.0f, 0.0f};
	const glm::vec3 yellow{1.0f, 1.0f, 0.0f};
	
	auto addPoint = [&](glm::vec4 point, glm::vec3 color = glm::vec3{1.0f, 1.0f, 1.0f})
	{
		const auto p = invProj * point;
		t_Vertices->pos = p.xyz() / p.w;
		t_Vertices->color = color;
		++t_Vertices;
	};

	addPoint(glm::vec4{0, 0,  -1, 1}); // c 0
	addPoint(glm::vec4{0, 0,   1, 1}); // t 1
	
	addPoint(glm::vec4{-w, -h, -1, 1}, red); // n1 2
	addPoint(glm::vec4{w,  -h, -1, 1}, red); // n3 3
	addPoint(glm::vec4{-w,  h, -1, 1}, red); // n2 4
	addPoint(glm::vec4{w,   h, -1, 1}, red); // n4 5

	addPoint(glm::vec4{-w, -h, 1, 1}, red); // f1 6
	addPoint(glm::vec4{w,  -h, 1, 1}, red); // f3 7
	addPoint(glm::vec4{-w,  h, 1, 1}, red); // f2 8
	addPoint(glm::vec4{w,   h, 1, 1}, red); // f4 9

	addPoint(glm::vec4{w*0.7f,  h*1.1f, -1, 1}, cyan); // u1 10
	addPoint(glm::vec4{-w*0.7f, h*1.1f, -1, 1}, cyan); // u2 11
	addPoint(glm::vec4{0,       h*2.0f, -1, 1}, cyan); // u3 12

	addPoint(glm::vec4{-w, 0,  1, 1}); // cf1 13
	addPoint(glm::vec4{w,  0,  1, 1}); // cf2 14
	addPoint(glm::vec4{0,  -h, 1, 1}); // cf3 15
	addPoint(glm::vec4{0,  h,  1, 1}); // cf4 16

	addPoint(glm::vec4{-w, 0,  -1, 1}); // cn1 17
	addPoint(glm::vec4{w,  0,  -1, 1}); // cn2 18
	addPoint(glm::vec4{0,  -h, -1, 1}); // cn3 19
	addPoint(glm::vec4{0,  h,  -1, 1}); // cn4 20

	t_Vertices->pos = {0, 0, 0}; // p 21
	t_Vertices->color = {1.0f, 1.0f, 1.0f};
	++t_Vertices;


	t_Indices.insert(t_Indices.end(), {2, 3}); // n1-n2
	t_Indices.insert(t_Indices.end(), {3, 5}); // n2-n4
	t_Indices.insert(t_Indices.end(), {5, 4}); // n4-n3
	t_Indices.insert(t_Indices.end(), {4, 2}); // n3-n1

	t_Indices.insert(t_Indices.end(), {6, 7}); // f1-f2
	t_Indices.insert(t_Indices.end(), {7, 9}); // f2-f4
	t_Indices.insert(t_Indices.end(), {9, 8}); // f4-f3
	t_Indices.insert(t_Indices.end(), {8, 6}); // f3-f1


	t_Indices.insert(t_Indices.end(), {2, 6}); // n1-f1
	t_Indices.insert(t_Indices.end(), {3, 7}); // n2-f2
	t_Indices.insert(t_Indices.end(), {4, 8}); // n3-f3
	t_Indices.insert(t_Indices.end(), {5, 9}); // n4-f4

	
	t_Indices.insert(t_Indices.end(), {21, 2}); // p-n1
	t_Indices.insert(t_Indices.end(), {21, 3}); // p-n2
	t_Indices.insert(t_Indices.end(), {21, 4}); // p-n3
	t_Indices.insert(t_Indices.end(), {21, 5}); // p-n4

	t_Indices.insert(t_Indices.end(), {10, 11}); // u1-u2
	t_Indices.insert(t_Indices.end(), {11, 12}); // u2-u3
	t_Indices.insert(t_Indices.end(), {12, 10}); // u3-u1 
	
	t_Indices.insert(t_Indices.end(), {0,  1}); // c-t
	t_Indices.insert(t_Indices.end(), {21, 0}); // p-c

	t_Indices.insert(t_Indices.end(), {17, 18}); // cn1-cn2
	t_Indices.insert(t_Indices.end(), {19, 20}); // cn3-cn4

	t_Indices.insert(t_Indices.end(), {13, 14}); // cf1-cf2
	t_Indices.insert(t_Indices.end(), {15, 16}); // cf3-cf4

	return 0;
}

static GeometryInfo PointLightHelperInfo()
{
	auto sphereInfo = SphereGeometryInfo(lightSphere);
	sphereInfo.type = GT_POINGHTLIGHTHELPER;
	return sphereInfo;
}

static int PointLightHelperData(ColorVertex* t_Vertices, std::vector<uint32>& t_Indices, uint32 t_BaseIndex = 0)
{
	auto res = SphereGeometryData(lightSphere, t_Vertices, t_Indices, t_BaseIndex);
	auto info = SphereGeometryInfo(lightSphere);

	for (size_t i = 0; i < info.vertexCount; ++i)
	{
		(t_Vertices + i)->color = {1.0f, 1.0f, 0.0f};
	}


	return res;
}
