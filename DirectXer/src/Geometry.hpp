#pragma once


#include <vector>

struct CubeGeometryInfo
{
	float width{1.0};
	float height{1.0};
	float depth{1.0};
	float widthSegments{1.0};
	float heightSegments{1.0};
	float depthSegments{1.0};

};

int CubeGeometry(const CubeGeometryInfo& t_CubeInfo, std::vector<float>& t_Vertices, std::vector<size_t>& t_Indices)
{
	int numberOfVertices = 0;
	
	auto buildPlane = [&](char u,
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
                uint32_t a = numberOfVertices + ix + gridX1 * iy;
                uint32_t b = numberOfVertices + ix + gridX1 * (iy + 1);
                uint32_t c = numberOfVertices + (ix + 1) + gridX1 * (iy + 1);
                uint32_t d = numberOfVertices + (ix + 1) + gridX1 * iy;

				
                // t_Indices.insert(t_Indices.end(), { a, b, d, d, c, b });
                t_Indices.insert(t_Indices.end(), { d, b, a, d, c, b });
            }
        }

        numberOfVertices += vertexCounter;
    };

	// buildPlane(2, 1, 0, -1, -1, depth, height, width, depthSegments, heightSegments);
    // buildPlane(2, 1, 0, 1, -1, depth, height, -width, depthSegments, heightSegments);
    // buildPlane(0, 2, 1, 1, 1, width, depth, height, widthSegments, depthSegments);
    // buildPlane(0, 2, 1, 1, -1, width, depth, -height, widthSegments, depthSegments);
    // buildPlane(0, 1, 2, 1, -1, width, height, depth, widthSegments, heightSegments);
    // buildPlane(0, 1, 2, -1, -1, width, height, -depth, widthSegments, heightSegments);

    buildPlane(2, 1, 0, -1, -1, t_CubeInfo.depth, t_CubeInfo.height, t_CubeInfo.width, t_CubeInfo.depthSegments, t_CubeInfo.heightSegments);
    buildPlane(2, 1, 0, 1, -1, t_CubeInfo.depth, t_CubeInfo.height, -t_CubeInfo.width, t_CubeInfo.depthSegments, t_CubeInfo.heightSegments);
    buildPlane(0, 2, 1, 1, 1, t_CubeInfo.width, t_CubeInfo.depth, t_CubeInfo.height, t_CubeInfo.widthSegments, t_CubeInfo.depthSegments);
    buildPlane(0, 2, 1, 1, -1, t_CubeInfo.width, t_CubeInfo.depth, -t_CubeInfo.height, t_CubeInfo.widthSegments, t_CubeInfo.depthSegments);
    buildPlane(0, 1, 2, 1, -1, t_CubeInfo.width, t_CubeInfo.height, t_CubeInfo.depth, t_CubeInfo.widthSegments, t_CubeInfo.heightSegments);
    buildPlane(0, 1, 2, -1, -1, t_CubeInfo.width, t_CubeInfo.height, -t_CubeInfo.depth, t_CubeInfo.widthSegments, t_CubeInfo.heightSegments);
	
	return 0;
}
