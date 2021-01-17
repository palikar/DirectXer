#include "Glm.hpp"
#include "Math.hpp"
#include "Geometry.hpp"

static GeometryInfo AxisHelperInfo()
{
	// @Todo: This is a number that is know all the time!
	CylinderGeometry cylinderShape{0.005f, 0.005f, 0.25f, 10.0f};
	auto cylinderInfo = CylinderGeometryInfo(cylinderShape);
	return {cylinderInfo.vertexCount*3, cylinderInfo.indexCount*3};
}

static int AxisHelperData(std::vector<ColorVertex>& t_Vertices, std::vector<uint32>& t_Indices)
{

	// @Todo: We _know_ the numbers that are in here
	CylinderGeometry cylinderShape{0.005f, 0.005f, 0.25f, 10.0f};
	auto cylinderInfo = CylinderGeometryInfo(cylinderShape);

	// @Todo: We can make everything without this vector
	std::vector<ColorVertex> vertices;
	vertices.resize(cylinderInfo.vertexCount*3);

	CylinderGeometryData(cylinderShape, &vertices[cylinderInfo.vertexCount*0].pos.x, t_Indices, sizeof(ColorVertex), cylinderInfo.vertexCount * 0u);
	CylinderGeometryData(cylinderShape, &vertices[cylinderInfo.vertexCount*1].pos.x, t_Indices, sizeof(ColorVertex), cylinderInfo.vertexCount * 1u);
	CylinderGeometryData(cylinderShape, &vertices[cylinderInfo.vertexCount*2].pos.x, t_Indices, sizeof(ColorVertex), cylinderInfo.vertexCount * 2u);

	glm::mat4 transform(1);
	transform = glm::translate(transform, glm::vec3(0.0f, 0.25f * 0.5f, 0.0f));
	TransformVertices(transform, &vertices[cylinderInfo.vertexCount*0], cylinderInfo.vertexCount);
	
	transform = glm::mat4(1);
	transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(1, 0, 0));
	transform = glm::translate(transform, glm::vec3(0.0f, 0.25f * 0.5f, 0.0f));
	TransformVertices(transform, &vertices[cylinderInfo.vertexCount*1], cylinderInfo.vertexCount);

	transform = glm::mat4(1);
	transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(0, 0, 1));
	transform = glm::translate(transform, glm::vec3(0.0f, 0.25f * 0.5f, 0.0f));
	TransformVertices(transform, &vertices[cylinderInfo.vertexCount*2], cylinderInfo.vertexCount);
	
	for (size_t i = 0; i < cylinderInfo.vertexCount; ++i)
	{
		vertices[i].color = glm::vec3{0.0f, 1.0f, 0.0f};
	}

	for (size_t i = cylinderInfo.vertexCount; i < cylinderInfo.vertexCount*2; ++i)
	{
		vertices[i].color = glm::vec3{1.0f, 0.0f, 0.0f};
	}

	for (size_t i = cylinderInfo.vertexCount*2; i < cylinderInfo.vertexCount*3; ++i)
	{
		vertices[i].color = glm::vec3{0.0f, 0.0f, 1.0f};
	}
	
	t_Vertices.insert(t_Vertices.end(), vertices.begin(), vertices.end());
	
	return 0;
}
