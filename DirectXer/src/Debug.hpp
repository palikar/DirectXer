#include "Glm.hpp"
#include "Math.hpp"
#include "Geometry.hpp"

static GeometryInfo AxisHelperInfo()
{
	// @Todo: This is a number that is know all the time!
	CylinderGeometry cylinderShape{0.005f, 0.005f, 0.25f, 10.0f};
	auto cylinderInfo = CylinderGeometryInfo(cylinderShape);
	return {cylinderInfo.vertexCount*3, cylinderInfo.indexCount*3, GT_AXISHELPER};
}

static int AxisHelperData(ColorVertex* t_Vertices, std::vector<uint32>& t_Indices)
{

	// @Todo: We _know_ the numbers that are in here
	CylinderGeometry cylinderShape{0.005f, 0.005f, 0.25f, 10.0f};
	auto cylinderInfo = CylinderGeometryInfo(cylinderShape);


	CylinderGeometryData(cylinderShape, t_Vertices, t_Indices, cylinderInfo.vertexCount * 0u);
	CylinderGeometryData(cylinderShape, t_Vertices, t_Indices, cylinderInfo.vertexCount * 1u);
	CylinderGeometryData(cylinderShape, t_Vertices, t_Indices, cylinderInfo.vertexCount * 2u);

	glm::mat4 transform(1);
	transform = glm::translate(transform, glm::vec3(0.0f, 0.25f * 0.5f, 0.0f));
	TransformVertices(transform, t_Vertices, cylinderInfo.vertexCount);
	
	transform = glm::mat4(1);
	transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(1, 0, 0));
	transform = glm::translate(transform, glm::vec3(0.0f, 0.25f * 0.5f, 0.0f));
	TransformVertices(transform, &t_Vertices[cylinderInfo.vertexCount*1], cylinderInfo.vertexCount);

	transform = glm::mat4(1);
	transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(0, 0, 1));
	transform = glm::translate(transform, glm::vec3(0.0f, 0.25f * 0.5f, 0.0f));
	TransformVertices(transform, &t_Vertices[cylinderInfo.vertexCount*2], cylinderInfo.vertexCount);
	
	for (size_t i = 0; i < cylinderInfo.vertexCount; ++i)
	{
		t_Vertices[i].color = glm::vec3{0.0f, 1.0f, 0.0f};
	}

	for (size_t i = cylinderInfo.vertexCount; i < cylinderInfo.vertexCount*2; ++i)
	{
		t_Vertices[i].color = glm::vec3{1.0f, 0.0f, 0.0f};
	}

	for (size_t i = cylinderInfo.vertexCount*2; i < cylinderInfo.vertexCount*3; ++i)
	{
		t_Vertices[i].color = glm::vec3{0.0f, 0.0f, 1.0f};
	}
	
	
	return 0;
}
