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
    BulkVector<Vertex2D, Memory_2DRendering> Vertices;
    BulkVector<uint32, Memory_2DRendering> Indices;
    VertexBufferId vbo;
    IndexBufferId ibo;
    
    Vertex2D* CurrentVertex;
    uint32 CurrentVertexCount;

    TextureId TexSlots[MaxTextureSlots];
    uint8 CurrentTextureSlot;

	TopolgyType SceneTopology;

  public:

    void InitRenderer(Graphics* t_Graphics, Init2DParams t_Params);

    // @Note: Begin new rendering of a 2D canvas; the functions has to
    // be called each time before we want to issue a sequence of 2D
    // draw commands to the renderer; the method clears any previous
    // state and sets up the internal vertex and index buffers;
    void BeginScene(TopolgyType sceneTopology = TT_TRIANGLES);

    // @Note: Flush the queued up draw commands and perform the
    // neccessary draw calls; each scene will be executed in as few
    // draw calls as possible; after calling EndScene, the BeginScene
    // method has to be called again before drawing
    void EndScene();
    
    uint8 AttachTexture(TextureId t_Tex);
    
    void DrawQuad(glm::vec2 pos, glm::vec2 size, glm::vec4 color);
    void DrawTriangle(const glm::vec2 vertices[3], glm::vec4 color);
	void DrawFourPolygon(const glm::vec2 vertices[4], glm::vec4 color);
    void DrawCirlce(glm::vec2 pos, float radius, glm::vec4 color);
    void DrawImage(uint32 t_Id, glm::vec2 pos, glm::vec2 size);
    void DrawRoundedQuad(glm::vec2 pos, glm::vec2 size, glm::vec4 color, float radius);
    void DrawSubImage(uint32 t_Id, glm::vec2 pos, glm::vec2 size, glm::vec2 subPos, glm::vec2 subSize);
    void DrawText(std::string_view text, glm::vec2 pos, FontId typeface);

	void DrawLine(glm::vec2 from, glm::vec2 to, glm::vec4 color);
};

// @Todo: Pusing transform matrices
// @Todo: Drawing lines

struct SpriteSheetHolder
{
	struct SpriteSheet
	{
		glm::vec2 SubSize;
		glm::ivec2 GridSize;
		uint32 ImageIndex;
	};

	BulkVector<SpriteSheet, Memory_2DRendering> Sheets;
	Renderer2D* Gfx2D;

    void Init(size_t t_Size, Renderer2D* Gfx);

	uint32 PutSheet(uint32 t_ImageIndex, glm::vec2 t_Size, glm::ivec2 t_GridSize);

	void DrawSprite(size_t spiretSheet, int index, glm::vec2 pos, glm::vec2 size);

	void DrawSprite(uint32 spiretSheet, glm::ivec2 spirtePos, glm::vec2 pos, glm::vec2 size);
};


struct Color
{
	static inline glm::vec4 Black{0.0f, 0.0f, 0.0f, 1.0f};
    static inline glm::vec4 White{1.0f, 1.0f, 1.0f, 1.0f};
    static inline glm::vec4 Red{1.0f, 0.0f, 0.0f, 1.0f};
    static inline glm::vec4 Blue{0.0f, 1.0f, 0.0f, 1.0f};
    static inline glm::vec4 Green{0.0f, 0.0f, 1.0f, 1.0f};
    static inline glm::vec4 Cyan{0.0f, 1.0f, 1.0f, 1.0f};
    static inline glm::vec4 Yellow{1.0f, 1.0f, 0.0f, 1.0f};
    static inline glm::vec4 Magenta{1.0f, 0.0f, 1.0f, 1.0f};
    static inline glm::vec4 Orange{1, 0.5, 0, 1.0f};
    static inline glm::vec4 AquaMarine{127.0f/255.0f, 255.0f/255.0f, 212.0f/255.0f, 1.0f};
	static inline glm::vec4 DarkViolet{148.0f/255.0f, 0.0f, 211.0f/255.0f, 1.0f};
	static inline glm::vec4 Chartreuse{127.0f/255.0f, 255.0f/255.0f, 0.0f, 1.0f};
	static inline glm::vec4 HotPink{255.0f/255.0f, 105.0f/255.0f, 180.0f/255.0f, 1.0f};
	static inline glm::vec4 Gold{255.0f/255.0f, 215.0f/255.0f, 0.0f, 1.0f};
	static inline glm::vec4 PaleGreen{152.0f/255.0f, 251.0f/255.0f, 152.0f/255.0f, 1.0f};
	static inline glm::vec4 Teal{0.0f, 128.0f/255.0f, 128.0f/255.0f, 1.0f};
};
