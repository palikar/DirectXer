#pragma once

#include <Types.hpp>
#include <Glm.hpp>
#include <GraphicsCommon.hpp>
#include <Graphics.hpp>
#include <ImageLibrary.hpp>
#include <FontLibrary.hpp>
#include <Resources.hpp>

struct Init2DParams
{
	float32 Width;
	float32 Height;
};


/*
  @Note: The Renderer2D is essentially the 2d graphics library of the
  engine; it does some basic batching of the issued commands by
  generating some data at the time of calling and putting the data
  inside several vertices of a vertex buffer; the indices are also
  generated at call time; the actual drawing is performed when the
  scene is ened and everything is flushed to the GPU -- the buffers
  are updated, the GPU state is setup and the neeed draw calls are
  made;

  Currently the information about which primitive we are drawing is
  stored in the vertex itself and in the shaers we do different things
  depending on the information inside the vertex; this make the
  implementation simple but it is uclear whether this approach scales
  or whether it is too hard on the GPU; eventally we may split the
  primitives by type and perform a draw call only for primitives of
  certain type; this has its own problems like the fact that the
  painter algorithm may be violated; 

*/
    
class Renderer2D
{
  public:

    inline static const size_t TotalVertices = 2000;
    inline static const uint8 MaxTextureSlots = 3;

    Init2DParams Params;
    Graphics* Graph;
    ImageLibrary ImageLib;
    FontLibrary FontLib;

    // @Note: The CPU memory for the Vertex and Index buffers
    BulkVector<Vertex2D> Vertices;
    BulkVector<uint32> Indices;
    VertexBufferId vbo;
    IndexBufferId ibo;
    
    Vertex2D* CurrentVertex;
    uint32 CurrentVertexCount;

    TextureId TexSlots[MaxTextureSlots];
    uint8 CurrentTextureSlot;

  public:

    void InitRenderer(Graphics* t_Graphics, Init2DParams t_Params);

    // @Note: Begin new rendering of a 2D canvas; the functions has to
    // be called each time before we want to issue a sequence of 2D
    // draw commands to the renderer; the method clears any previous
    // state and sets up the internal vertex and index buffers;
    void BeginScene();

    // @Note: Flush the queued up draw commands and perform the
    // neccessary draw calls; each scene will be executed in as few
    // draw calls as possible; after calling EndScene, the BeginScene
    // method has to be called again before drawing
    void EndScene();
    
    uint8 AttachTexture(TextureId t_Tex);
    
    void DrawQuad(glm::vec2 pos, glm::vec2 size, glm::vec4 color);
    void DrawTriangle(glm::vec2 vertices[3], glm::vec4 color);
    void DrawCirlce(glm::vec2 pos, float radius, glm::vec4 color);
    void DrawImage(uint32 t_Id, glm::vec2 pos, glm::vec2 size);
    void DrawRoundedQuad(glm::vec2 pos, glm::vec2 size, glm::vec4 color, float radius);
    void DrawSubImage(uint32 t_Id, glm::vec2 pos, glm::vec2 size, glm::vec2 subPos, glm::vec2 subSize);
    void DrawText(std::string_view text, glm::vec2 pos, FontId typeface);
};

// @Todo: Pusing transform matrices
// @Todo: Filling trangles
// @Todo: Drawing lines


struct SpriteSheetHolder
{
	struct SpriteSheet
	{
		glm::vec2 SubSize;
		glm::ivec2 GridSize;
		uint32 ImageIndex;
	};

	BulkVector<SpriteSheet> Sheets;
	Renderer2D* Gfx2D;

    void Init(size_t t_Size, Renderer2D* Gfx)
	{
		Sheets.reserve(t_Size);
		Gfx2D = Gfx;
	}

	uint32 PutSheet(uint32 t_ImageIndex, glm::vec2 t_Size, glm::ivec2 t_GridSize)
	{
		SpriteSheet sheet;
		sheet.SubSize = glm::vec2{ t_Size.x / t_GridSize.x, t_Size.y / t_GridSize.y };
		sheet.GridSize = t_GridSize;
		sheet.ImageIndex = t_ImageIndex;
		Sheets.push_back(sheet);

		return (uint32)Sheets.size() - 1;
	}

	void DrawSprite(size_t spiretSheet, int index, glm::vec2 pos, glm::vec2 size)
	{
		const auto& sheet = Sheets[spiretSheet];

		const int x = index % sheet.GridSize.x;
		const int y = index / sheet.GridSize.x;

		Gfx2D->DrawSubImage(sheet.ImageIndex, pos, size, { x * sheet.SubSize.x, y * sheet.SubSize.y }, sheet.SubSize);
	}

	void DrawSprite(uint32 spiretSheet, glm::ivec2 spirtePos, glm::vec2 pos, glm::vec2 size)
	{
		const auto& sheet = Sheets[spiretSheet];

		const int x = spirtePos.x;
		const int y = spirtePos.y;

		Gfx2D->DrawSubImage(sheet.ImageIndex, pos, size, { x * sheet.SubSize.x, y * sheet.SubSize.y }, sheet.SubSize);
	}
};
