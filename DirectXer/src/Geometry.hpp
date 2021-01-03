#pragma once


#include <vector>
#include <cmath>

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
	size_t width{1};
	size_t height{1};
	float width_segments{1.0};
	float height_segments{1.0};
};

struct GeometryInfo
{
	size_t vertexCount{0};
	size_t indexCount{0};
};

GeometryInfo CubeGeometryInfo(const CubeGeometry& t_CubeInfo)
{

	size_t vertices = (size_t)((t_CubeInfo.depthSegments + 1) * (t_CubeInfo.heightSegments + 1));
	vertices += (size_t)((t_CubeInfo.widthSegments + 1) * (t_CubeInfo.depthSegments  + 1));
	vertices += (size_t)((t_CubeInfo.widthSegments + 1) * (t_CubeInfo.heightSegments + 1));
	vertices *= 2;
	vertices *= 3;

	size_t indices = (size_t)(t_CubeInfo.depthSegments * t_CubeInfo.heightSegments);
	indices += (size_t)(t_CubeInfo.widthSegments * t_CubeInfo.depthSegments);
	indices += (size_t)(t_CubeInfo.widthSegments * t_CubeInfo.heightSegments);
	indices *= 2;
	indices *= 6;

	return {vertices, indices};
}

int CubeGeometry(const CubeGeometry& t_CubeInfo, std::vector<float>& t_Vertices, std::vector<size_t>& t_Indices)
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


GeometryInfo PlaneGeometryInfo(const PlaneGeometry& t_PlaneInfo)
{
	size_t vertices = (size_t)((std::floor(t_PlaneInfo.width_segments) + 1) * (std::floor(t_PlaneInfo.height_segments) + 1));
	vertices *= 3;

	size_t indices = (size_t)((std::floor(t_PlaneInfo.width_segments)) * (std::floor(t_PlaneInfo.height_segments)));
	indices *= 6;

	return {vertices, indices};
}

int PlaneGeometry(const PlaneGeometry& t_PlaneInfo, std::vector<float>& t_Vertices, std::vector<size_t>& t_Indices)
{
	const auto half_width  = t_PlaneInfo.width / 2.0f;
    const auto half_height = t_PlaneInfo.height / 2.0f;

    const auto grid_x          = (std::floor(t_PlaneInfo.width_segments));
    const auto grid_y          = (std::floor(t_PlaneInfo.height_segments));
    const auto grid_x1         = grid_x + 1;
    const auto grid_y1         = grid_y + 1;
    const float segment_width  = t_PlaneInfo.width / grid_x;
    const float segment_height = t_PlaneInfo.height / grid_y;

    for (size_t iy = 0; iy < grid_y1; iy++)
    {
        auto y = iy * segment_height - half_height;
        for (size_t ix = 0; ix < grid_x1; ix++)
        {
            auto x = ix * segment_width - half_width;

            t_Vertices.insert(t_Vertices.end(), { x, -y, 0 });
            // normals.insert(normals.end(), { 0, 0, -1 });
            // uv.insert(uv.end(), { ix / grid_x, 1 - (iy / grid_y) });
        }
    }


    for (size_t iy = 0; iy < grid_y; iy++)
    {
        for (size_t ix = 0; ix < grid_x; ix++)
        {
            uint32_t a = static_cast<size_t>(ix + grid_x1 * iy);
            uint32_t b = static_cast<size_t>(ix + grid_x1 * (iy + 1));
            uint32_t c = static_cast<size_t>((ix + 1) + grid_x1 * (iy + 1));
            uint32_t d = static_cast<size_t>((ix + 1) + grid_x1 * iy);

            t_Indices.insert(t_Indices.end(), { a, b, d, b, c, d });
        }
    }

	return 0;
}
