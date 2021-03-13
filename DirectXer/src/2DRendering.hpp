#pragma once

#include "Types.hpp"
#include "Glm.hpp"
#include "GraphicsCommon.hpp"
#include "Graphics.hpp"
#include "Memory.hpp"


#include <stb_rect_pack.h>


struct Init2DParams
{
	float32 Width;
	float32 Height;
};

struct Image
{
	TextureObject TexHandle;
	glm::vec2 ScreenPos;
	glm::vec2 ScreenSize;
};

struct ImageAtlas
{
	TextureObject TexHandle;
	glm::vec2 Size;
	stbrp_context RectContext;
};

class ImageLibraryBuilder
{
public:

	std::vector<std::string> ImageFilePaths;

	uint32 PutImage(std::string t_Path)
	{
		ImageFilePaths.push_back(t_Path);
		return (uint32)ImageFilePaths.size() - 1;
	}

};

class ImageLibrary
{
  public:
	std::vector<Image> Images;
	std::string_view ResourcesPath;
	Graphics* Gfx;
	MemoryArena fileArena;
	std::vector<ImageAtlas> Atlases;

	const static inline int RectsCount = 1024 / 2;

	void Init(Graphics* Gfx, std::string_view t_Path)
	{
		Atlases.reserve(10);
		
		this->Gfx = Gfx;
		InitAtlas({ 1024, 1024 });
		this->ResourcesPath = t_Path;
	}


	ImageAtlas InitAtlas(glm::vec2 t_Size)
	{
		auto space = malloc(sizeof(stbrp_node) * RectsCount);
		
		ImageAtlas newAtlas;
		newAtlas.TexHandle = Gfx->createTexture((uint16)t_Size.x, (uint16)t_Size.y, TF_RGBA, nullptr, 0);
		newAtlas.Size = t_Size;
		Atlases.push_back(newAtlas);

		// @Note: 8Kb Per atlas for tect packing; maybe we can bump this to 16KB for best rect packing results		
		stbrp_init_target(&Atlases.back().RectContext, (int)t_Size.x, (int)t_Size.y, (stbrp_node*)space, RectsCount);

		return newAtlas;
	}

	void Build(ImageLibraryBuilder t_Builder)
	{
		fileArena = Memory::GetTempArena(Megabytes(16));
		fmt::basic_memory_buffer<char, 512> buf;

		stbi_set_flip_vertically_on_load(0);
		
		for (const auto& imagePath : t_Builder.ImageFilePaths)
		{
			fmt::format_to(buf, "{}/{}", ResourcesPath, imagePath);
			DXLOG("[RES] Loading {}", buf.c_str());
			ReadWholeFile(buf.c_str(), fileArena);
			int width, height, channels;
			unsigned char* data = stbi_load_from_memory((unsigned char*)fileArena.Memory, (int)fileArena.Size, &width, &height, &channels, 0);
			fileArena.Reset();
			buf.clear();

			stbrp_rect rect;
			rect.w = (stbrp_coord)width;
			rect.h = (stbrp_coord)height;
				
			for (auto& atlas: Atlases)
			{
				stbrp_pack_rects(&atlas.RectContext, &rect, 1);
				if (rect.was_packed == 0) continue;

				Rectangle2D updateRect{{rect.x, rect.y},{rect.w, rect.h}};
				Gfx->updateTexture(atlas.TexHandle, updateRect, data);
				Images.push_back({ atlas.TexHandle, {rect.x / 1024.0f, rect.y / 1024.0f}, {rect.w / 1024.0f, rect.h / 1024.0f} });
				break;
			}

			
			if (rect.was_packed != 0) continue;

			auto atlas = InitAtlas({1024, 1024});
			stbrp_pack_rects(&atlas.RectContext, &rect, 1);
			assert(rect.was_packed != 0);


			Rectangle2D updateRect { {rect.x, rect.y}, { rect.w, rect.h }};
			Gfx->updateTexture(atlas.TexHandle, updateRect, data);
			Images.push_back({ atlas.TexHandle, {rect.x / 1024.0f, rect.y / 1024.0f}, {rect.w / 1024.0f, rect.h / 1024.0f} });
			
		}

	}

	ImageAtlas GetOrCreateAtlas(glm::vec2 t_Size)
	{

		
	}
	


	Image GetImage(uint32 t_id)
	{
		return Images[t_id];
	}
	
};

class Renderer2D
{
public:

	asl::BulkVector<Vertex2D> Vertices;
	asl::BulkVector<uint32> Indices;
	Vertex2D* CurrentVertex;
	ImageLibrary Images;

	uint32 CurrentVertexCount;

	size_t TotalVertices = 2000;

	VBObject vbo;
	IBObject ibo;

	TextureObject Tex1;

	Graphics* Graph;
	Init2DParams Params;

	void InitRenderer(Graphics* t_Graphics, Init2DParams t_Params)
	{
		Graph = t_Graphics;
		Params = t_Params;

		
		Vertices.resize(TotalVertices);
		Indices.resize(TotalVertices * 3);
		
		CurrentVertex = &Vertices[0];

		vbo = Graph->createVertexBuffer(sizeof(Vertex2D), nullptr, (uint32)(sizeof(Vertex2D) * TotalVertices), true);
		ibo = Graph->createIndexBuffer(nullptr, (uint32)(sizeof(uint32) * TotalVertices * 3), true);
	}

	void BeginScene()
	{
		Graph->VertexShaderCB.projection = glm::transpose(glm::ortho(0.0f, Params.Width,  Params.Height, 0.0f));
		Graph->VertexShaderCB.model = glm::mat4(1.0f);

		CurrentVertexCount = 0;
		
		Indices.clear();
		Vertices.clear();

		CurrentVertex = &Vertices[0];		
	}

	void EndScene()
	{
		Graph->setShaderConfiguration(SC_2D_RECT);

		Graph->setIndexBuffer(ibo);
		Graph->setVertexBuffer(vbo);

		Graph->updateVertexBuffer(vbo, Vertices.data(), CurrentVertexCount*sizeof(Vertex2D));
		Graph->updateIndexBuffer(ibo, Indices.data(), 3u*CurrentVertexCount*sizeof(uint32));
		Graph->updateCBs();

		Graph->bindTexture(0, Tex1);

		Graph->drawIndex(Graphics::TT_TRIANGLES, 3u * CurrentVertexCount, 0, 0);

	}
	
	void DrawQuad(glm::vec2 pos, glm::vec2 size, glm::vec4 color)
	{
		CurrentVertex->pos = pos;
		CurrentVertex->color = color;
		CurrentVertex->type = 1;
		++CurrentVertex;

		CurrentVertex->pos = glm::vec2{pos.x + size.x, pos.y};
		CurrentVertex->color = color;
		CurrentVertex->type = 1;
		++CurrentVertex;

		CurrentVertex->pos = glm::vec2{pos.x, pos.y + size.y};
		CurrentVertex->color = color;
		CurrentVertex->type = 1;
		++CurrentVertex;

		CurrentVertex->pos = pos + size;
		CurrentVertex->color = color;
		CurrentVertex->type = 1;
		++CurrentVertex;

		Indices.insert(Indices.end(), { CurrentVertexCount , CurrentVertexCount + 1, CurrentVertexCount + 2,
										CurrentVertexCount + 2 , CurrentVertexCount + 1, CurrentVertexCount + 3});

		CurrentVertexCount += 4;
		
	}

	void DrawCirlce(glm::vec2 pos, float radius, glm::vec4 color)
	{
		const float r2 = radius;
		
		CurrentVertex->pos = glm::vec2{pos.x - r2, pos.y - r2};
		CurrentVertex->color = color;
		CurrentVertex->additional = glm::vec3{radius, -1.0f, -1.0f};
		CurrentVertex->type = 2;
		++CurrentVertex;

		CurrentVertex->pos = glm::vec2{pos.x - r2, pos.y + r2};
		CurrentVertex->color = color;
		CurrentVertex->additional = glm::vec3{radius, -1.0f, 1.0f};
		CurrentVertex->type = 2;
		++CurrentVertex;

		CurrentVertex->pos = glm::vec2{pos.x + r2, pos.y - r2};
		CurrentVertex->color = color;
		CurrentVertex->additional = glm::vec3{radius, 1.0f, -1.0f};
		CurrentVertex->type = 2;
		++CurrentVertex;

		CurrentVertex->pos = glm::vec2{pos.x + r2, pos.y + r2};
		CurrentVertex->color = color;
		CurrentVertex->additional = glm::vec3{radius, 1.0f, 1.0f};
		CurrentVertex->type = 2;
		++CurrentVertex;

		Indices.insert(Indices.end(), { CurrentVertexCount , CurrentVertexCount + 1, CurrentVertexCount + 2,
										CurrentVertexCount + 2 , CurrentVertexCount + 1, CurrentVertexCount + 3});

		CurrentVertexCount += 4;
		
	}

	void DrawImage(uint32 t_Id, glm::vec2 pos, glm::vec2 size)
	{
		assert(t_Id < Images.Images.size());
		const auto& screenImage = Images.Images[t_Id];

		Tex1 = screenImage.TexHandle;
		
		CurrentVertex->pos = pos;
		CurrentVertex->type = 3;
		CurrentVertex->additional.x = screenImage.ScreenPos.x;
		CurrentVertex->additional.y = screenImage.ScreenPos.y;
		++CurrentVertex;

		CurrentVertex->pos = glm::vec2{pos.x + size.x, pos.y};
		CurrentVertex->type = 3;
		CurrentVertex->additional.x = screenImage.ScreenPos.x + screenImage.ScreenSize.x;
		CurrentVertex->additional.y = screenImage.ScreenPos.y;
		++CurrentVertex;

		CurrentVertex->pos = glm::vec2{pos.x, pos.y + size.y};
		CurrentVertex->type = 3;
		CurrentVertex->additional.x = screenImage.ScreenPos.x;
		CurrentVertex->additional.y = screenImage.ScreenPos.y + screenImage.ScreenSize.y;
		++CurrentVertex;

		CurrentVertex->pos = pos + size;
		CurrentVertex->type = 3;
		CurrentVertex->additional.x = screenImage.ScreenPos.x + screenImage.ScreenSize.x;
		CurrentVertex->additional.y = screenImage.ScreenPos.y + screenImage.ScreenSize.y;
		++CurrentVertex;


		Indices.insert(Indices.end(), { CurrentVertexCount , CurrentVertexCount + 1, CurrentVertexCount + 2,
										CurrentVertexCount + 2 , CurrentVertexCount + 1, CurrentVertexCount + 3});

		CurrentVertexCount += 4;
		
	}

};

// Drawing textured Quad
	
// Drawing rounded Quad

// Drawing rounded textured Quad

// Drawing textured circle


// Drawing Lines

// Pusing transform matrices?
