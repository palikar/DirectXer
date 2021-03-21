#pragma once

#include "Types.hpp"
#include "Glm.hpp"
#include "GraphicsCommon.hpp"
#include "Graphics.hpp"
#include "ImageLibrary.hpp"
#include "FontLibrary.hpp"
#include "Resources.hpp"


struct Init2DParams
{
	float32 Width;
	float32 Height;
};

class Renderer2D
{
  public:

	BulkVector<Vertex2D> Vertices;
	BulkVector<uint32> Indices;
	Vertex2D* CurrentVertex;
	ImageLibrary ImageLib;
	FontLibrary FontLib;

	uint32 CurrentVertexCount;

	inline static const size_t TotalVertices = 2000;
	inline static const uint8 MaxTextureSlots = 3;

	VBObject vbo;
	IBObject ibo;

	TextureObject TexSlots[MaxTextureSlots];
	uint8 CurrentTextureSlot;

	Graphics* Graph;
	Init2DParams Params;

  public:

	
	void InitRenderer(Graphics* t_Graphics, Init2DParams t_Params);

	void BeginScene();
	void EndScene();

	uint8 AttachTexture(TextureObject t_Tex);

	void DrawQuad(glm::vec2 pos, glm::vec2 size, glm::vec4 color);
	void DrawCirlce(glm::vec2 pos, float radius, glm::vec4 color);
	void DrawImage(uint32 t_Id, glm::vec2 pos, glm::vec2 size);
	void DrawRoundedQuad(glm::vec2 pos, glm::vec2 size, glm::vec4 color, float radius);
	void DrawText(std::string_view text, glm::vec2 pos, uint8 typeface);
};

// Drawing Lines

// Pusing transform matrices
