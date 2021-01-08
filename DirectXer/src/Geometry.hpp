#pragma once

#include "Types.h"

#include <vector>
#include <cmath>

inline constexpr float PI = 3.14159265358979323846;

struct CubeGeometry
{
	float width{1.0};
	float height{1.0};
	float depth{1.0};
	float widthSegments{1.0};
	float heightSegments{1.0};
	float depthSegments{1.0};

};

struct PlaneGeometry
{
	uint32 width{1};
	uint32 height{1};
	float width_segments{1.0};
	float height_segments{1.0};
};

struct SphereGeometry
{
	float radius{1.0};
	float width_segments{8.0};
	float height_segments{8.0};
	float phi_start{0.0};
	float phi_length{2.0f*PI};
	float theta_start{0.0};
	float theta_length{1.0f*PI};
};

struct GeometryInfo
{
	uint32 vertexCount{0};
	uint32 indexCount{0};
};

inline GeometryInfo CubeGeometryInfo(const CubeGeometry& t_CubeInfo)
{

	uint32 vertices = (uint32)((t_CubeInfo.depthSegments + 1) * (t_CubeInfo.heightSegments + 1));
	vertices += (uint32)((t_CubeInfo.widthSegments + 1) * (t_CubeInfo.depthSegments  + 1));
	vertices += (uint32)((t_CubeInfo.widthSegments + 1) * (t_CubeInfo.heightSegments + 1));
	vertices *= 2;
	vertices *= 3;

	uint32 indices = (uint32)(t_CubeInfo.depthSegments * t_CubeInfo.heightSegments);
	indices += (uint32)(t_CubeInfo.widthSegments * t_CubeInfo.depthSegments);
	indices += (uint32)(t_CubeInfo.widthSegments * t_CubeInfo.heightSegments);
	indices *= 2;
	indices *= 6;

	return {vertices, indices};
}

inline int CubeGeometry(const CubeGeometry& t_CubeInfo, std::vector<float>& t_Vertices, std::vector<uint32>& t_Indices)
{
	int numberOfVertices = 0;
	
	const auto buildPlane = [&](char u,
	char v,
	char w,
	float udir,
	float vdir,
	float t_width,
	float t_height,
	float t_depth,
	float gridX,
	float gridY) {
        float segmentWidth  = t_width / gridX;
        float segmentHeight = t_height / gridY;
        float widthHalf     = t_width / 2;
        float heightHalf    = t_height / 2;
        float depthHalf     = t_depth / 2;
        float gridX1        = gridX + 1;
        float gridY1        = gridY + 1;
        int vertexCounter   = 0;
        int ix, iy;

        for (iy = 0; iy < gridY1; iy++)
        {
            float y = iy * segmentHeight - heightHalf;
            for (ix = 0; ix < gridX1; ix++)
            {
                float x = ix * segmentWidth - widthHalf;

				// vertex[u] = x * udir;
                // vertex[v] = y * vdir;
                // vertex[w] = depthHalf;

                // vertex[3 + u] = 0;
                // vertex[3 + v] = 0;
                // vertex[3 + w] = t_depth > 0 ? 1 : -1;

                // vertex[6] = (ix / gridX);
                // vertex[7] = (1 - (iy / gridY));

				float vert[3];
				*(vert + u) = x * udir;
				*(vert + v) = y * vdir;
				*(vert + w) = depthHalf;
				
                t_Vertices.insert(t_Vertices.end(), {vert[0], vert[1], vert[2]});
                // normals.insert(normals.end(), { vertex[3], vertex[4], vertex[5] });
                // uv.insert(uv.end(), { ix / gridX, 1 - (iy / gridY) });
                // vertecies.push_back(vertex);

                vertexCounter += 1;
            };
        }

        for (iy = 0; iy < gridY; iy++)
        {
            for (ix = 0; ix < gridX; ix++)
            {
                uint32_t a = (uint32_t)(numberOfVertices + ix + gridX1 * iy);
                uint32_t b = (uint32_t)(numberOfVertices + ix + gridX1 * (iy + 1));
                uint32_t c = (uint32_t)(numberOfVertices + (ix + 1) + gridX1 * (iy + 1));
                uint32_t d = (uint32_t)(numberOfVertices + (ix + 1) + gridX1 * iy);

				t_Indices.insert(t_Indices.end(), { d, b, a, d, c, b });
            }
        }

        numberOfVertices += vertexCounter;
    };

	buildPlane(2, 1, 0, -1, -1, t_CubeInfo.depth, t_CubeInfo.height, t_CubeInfo.width, t_CubeInfo.depthSegments, t_CubeInfo.heightSegments);
    buildPlane(2, 1, 0, 1, -1, t_CubeInfo.depth, t_CubeInfo.height, -t_CubeInfo.width, t_CubeInfo.depthSegments, t_CubeInfo.heightSegments);
    buildPlane(0, 2, 1, 1, 1, t_CubeInfo.width, t_CubeInfo.depth, t_CubeInfo.height, t_CubeInfo.widthSegments, t_CubeInfo.depthSegments);
	buildPlane(0, 2, 1, 1, -1, t_CubeInfo.width, t_CubeInfo.depth, -t_CubeInfo.height, t_CubeInfo.widthSegments, t_CubeInfo.depthSegments);
    buildPlane(0, 1, 2, 1, -1, t_CubeInfo.width, t_CubeInfo.height, t_CubeInfo.depth, t_CubeInfo.widthSegments, t_CubeInfo.heightSegments);
    buildPlane(0, 1, 2, -1, -1, t_CubeInfo.width, t_CubeInfo.height, -t_CubeInfo.depth, t_CubeInfo.widthSegments, t_CubeInfo.heightSegments);
	
	return 0;
}

inline GeometryInfo PlaneGeometryInfo(const PlaneGeometry& t_PlaneInfo)
{
	uint32 vertices = (uint32)((std::floor(t_PlaneInfo.width_segments) + 1) * (std::floor(t_PlaneInfo.height_segments) + 1));
	vertices *= 3;

	uint32 indices = (uint32)((std::floor(t_PlaneInfo.width_segments)) * (std::floor(t_PlaneInfo.height_segments)));
	indices *= 6;

	return {vertices, indices};
}

inline int PlaneGeometry(const PlaneGeometry& t_PlaneInfo, std::vector<float>& t_Vertices, std::vector<uint32>& t_Indices)
{
	const auto half_width  = t_PlaneInfo.width / 2.0f;
    const auto half_height = t_PlaneInfo.height / 2.0f;

    const auto grid_x          = (std::floor(t_PlaneInfo.width_segments));
    const auto grid_y          = (std::floor(t_PlaneInfo.height_segments));
    const auto grid_x1         = grid_x + 1;
    const auto grid_y1         = grid_y + 1;
    const float segment_width  = t_PlaneInfo.width / grid_x;
    const float segment_height = t_PlaneInfo.height / grid_y;

    for (uint32 iy = 0; iy < grid_y1; iy++)
    {
        auto y = iy * segment_height - half_height;
        for (uint32 ix = 0; ix < grid_x1; ix++)
        {
            auto x = ix * segment_width - half_width;

            t_Vertices.insert(t_Vertices.end(), { x, -y, 0 });
            // normals.insert(normals.end(), { 0, 0, -1 });
            // uv.insert(uv.end(), { ix / grid_x, 1 - (iy / grid_y) });
        }
    }


    for (uint32 iy = 0; iy < grid_y; iy++)
    {
        for (uint32 ix = 0; ix < grid_x; ix++)
        {
            uint32_t a = static_cast<uint32>(ix + grid_x1 * iy);
            uint32_t b = static_cast<uint32>(ix + grid_x1 * (iy + 1));
            uint32_t c = static_cast<uint32>((ix + 1) + grid_x1 * (iy + 1));
            uint32_t d = static_cast<uint32>((ix + 1) + grid_x1 * iy);

            t_Indices.insert(t_Indices.end(), { a, b, d, b, c, d });
        }
    }

	return 0;
}

inline GeometryInfo SphereGeometryInfo(const SphereGeometry& t_SphereInfo)
{

	const float width_segments  = std::max(3.0f, std::floor(t_SphereInfo.width_segments));
    const float height_segments = std::max(2.0f, std::floor(t_SphereInfo.height_segments));
	const float theta_end = std::min(t_SphereInfo.theta_start + t_SphereInfo.theta_length, PI);
	
	uint32 vertices = (uint32)((width_segments + 1) * (height_segments + 1));
	vertices *= 3;

	
	uint32 indices = 0;
	int ix, iy;
    for (iy = 0; iy < height_segments; ++iy)
    {

        for (ix = 0; ix < width_segments; ++ix)
        {

			if (iy != 0 || t_SphereInfo.theta_start > 0)
            {
                indices+=3;
            }

            if (iy != height_segments - 1 || theta_end < PI)
            {
                indices+=3;
            }
        }
    }

	return {vertices, indices};
}

inline int SphereGeometry(const SphereGeometry& t_SphereInfo, std::vector<float>& t_Vertices, std::vector<uint32>& t_Indices)
{

	float radius = std::max(t_SphereInfo.radius, 1.0f);

    float width_segments  = std::max(3.0f, std::floor(t_SphereInfo.width_segments));
    float height_segments = std::max(2.0f, std::floor(t_SphereInfo.height_segments));

    float theta_end = std::min(t_SphereInfo.theta_start + t_SphereInfo.theta_length, PI);
    int ix, iy;
    uint32 index = 0;
    std::vector<std::vector<uint32>> grid;

    for (iy = 0; iy <= height_segments; ++iy)
    {
        std::vector<uint32> verticesRow;
        float v = iy / height_segments;
		
        float uOffset = 0;
        if (iy == 0 && t_SphereInfo.theta_start == 0)
        {
            uOffset = 0.5f / width_segments;
        }
        else if (iy == height_segments && theta_end == PI)
        {
            uOffset = -0.5f / width_segments;
        }

        for (ix = 0; ix <= width_segments; ix++)
        {
            float u       = ix / width_segments;
            const float x = -radius * std::cos(t_SphereInfo.phi_start + u * t_SphereInfo.phi_length)
                            * std::sin(t_SphereInfo.theta_start + v * t_SphereInfo.theta_length);
            const float y = radius * std::cos(t_SphereInfo.theta_start + v * t_SphereInfo.theta_length);
            const float z = radius * std::sin(t_SphereInfo.phi_start + u * t_SphereInfo.phi_length)
                            * std::sin(t_SphereInfo.theta_start + v * t_SphereInfo.theta_length);

            t_Vertices.insert(t_Vertices.end(), { x, y, z });

            // auto norm = glm::normalize(glm::vec3(x, y, z));
            // normals.insert(normals.end(), { norm.x, norm.y, norm.z });
            // uv.insert(uv.end(), { u + uOffset, 1 - v });

            verticesRow.push_back(index++);
        }

        grid.push_back(verticesRow);
    }

    for (iy = 0; iy < height_segments; ++iy)
    {

        for (ix = 0; ix < width_segments; ++ix)
        {

            uint32_t a = grid[iy][ix + 1];
            uint32_t b = grid[iy][ix];
            uint32_t c = grid[iy + 1][ix];
            uint32_t d = grid[iy + 1][ix + 1];

            if (iy != 0 || t_SphereInfo.theta_start > 0)
            {
                t_Indices.insert(t_Indices.end(), { d, b, a });
            }

            if (iy != height_segments - 1 || theta_end < PI)
            {
                t_Indices.insert(t_Indices.end(), { d, c, b });
            }
        }
    }

	return 0;
}
