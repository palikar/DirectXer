#pragma once

#include "Types.hpp"
#include "Glm.hpp"
#include "GraphicsCommon.hpp"
#include "Graphics.hpp"
#include "ImageLibrary.hpp"
#include "2DRendering.hpp"


void Renderer2D::InitRenderer(Graphics* t_Graphics, Init2DParams t_Params)
{
	Graph = t_Graphics;
	Params = t_Params;
	CurrentTextureSlot = 0;

	ImageLib.Init(t_Graphics);
	FontLib.Init(t_Graphics);
			
	Vertices.resize(TotalVertices);
	Indices.resize(TotalVertices * 3);
		
	CurrentVertex = &Vertices[0];

	vbo = NextVertexBufferId();
	Graph->CreateVertexBuffer(vbo, sizeof(Vertex2D), nullptr, (uint32)(sizeof(Vertex2D) * TotalVertices), true);
	ibo = NextIndexBufferId();
	Graph->CreateIndexBuffer(ibo , nullptr, (uint32)(sizeof(uint32) * TotalVertices * 3), true);
}

void Renderer2D::BeginScene()
{
	CurrentVertexCount = 0;
		
	Indices.clear();
	Vertices.clear();

	Vertices.resize(TotalVertices);

	CurrentTextureSlot = 0;
	for (size_t i = 0; i < MaxTextureSlots; ++i)
	{
		TexSlots[i] = {0};
	}

	CurrentVertex = &Vertices[0];		
}

void Renderer2D::EndScene()
{
	Graph->VertexShaderCB.projection = glm::transpose(glm::ortho(0.0f, Params.Width, Params.Height, 0.0f));
	Graph->VertexShaderCB.model = glm::mat4(1.0f);

	Graph->SetShaderConfiguration(SC_2D_RECT);

	Graph->BindIndexBuffer(ibo);
	Graph->BindVertexBuffer(vbo);

	Graph->UpdateVertexBuffer(vbo, Vertices.data(), CurrentVertexCount * sizeof(Vertex2D));
	Graph->UpdateIndexBuffer(ibo, Indices.data(), 3u * CurrentVertexCount * sizeof(uint32));
	Graph->UpdateCBs();

	Graph->SetBlendingState(BS_AlphaBlending);

	for (uint32 i = 0; i < CurrentTextureSlot; ++i)
	{
		Graph->BindTexture(i, TexSlots[i]);
	}


	Graph->DrawIndex(TT_TRIANGLES, (uint32)Indices.size(), 0u, 0u);

}

uint8 Renderer2D::AttachTexture(TextureId t_Tex)
{
	for (uint8 i = 0; i < MaxTextureSlots; ++i)
	{
		if(TexSlots[i] == t_Tex) return i;
	}
			
	if (CurrentTextureSlot >= MaxTextureSlots)
	{
		EndScene();
		BeginScene();
	}
	TexSlots[CurrentTextureSlot++] = t_Tex;

	return CurrentTextureSlot - 1;
}
	
void Renderer2D::DrawQuad(glm::vec2 pos, glm::vec2 size, glm::vec4 color)
{
	if (CurrentVertexCount + 4 >= TotalVertices)
	{
		EndScene();
		BeginScene();
	}
		
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

void Renderer2D::DrawRoundedQuad(glm::vec2 pos, glm::vec2 size, glm::vec4 color, float radius)
{
	
	if (CurrentVertexCount + 4 >= TotalVertices)
	{
		EndScene();
		BeginScene();
	}

	radius /= size.x;
	// radius /= Params.Width;
	// radius *= 2;
	radius = (radius * 2.0f) - 1.0f;

	CurrentVertex->pos = pos;
	CurrentVertex->color = color;
	CurrentVertex->type = 4;
	CurrentVertex->additional = glm::vec3{radius, -1.0f, -1.0f};
	++CurrentVertex;

	CurrentVertex->pos = glm::vec2{pos.x + size.x, pos.y};
	CurrentVertex->color = color;
	CurrentVertex->type = 4;
	CurrentVertex->additional = glm::vec3{radius, -1.0f, 1.0f};
	++CurrentVertex;

	CurrentVertex->pos = glm::vec2{pos.x, pos.y + size.y};
	CurrentVertex->color = color;
	CurrentVertex->type = 4;
	CurrentVertex->additional = glm::vec3{radius, 1.0f, -1.0f};
	++CurrentVertex;

	CurrentVertex->pos = pos + size;
	CurrentVertex->color = color;
	CurrentVertex->type = 4;
	CurrentVertex->additional = glm::vec3{radius, 1.0f, 1.0f};
	++CurrentVertex;
	
	Indices.insert(Indices.end(), { CurrentVertexCount , CurrentVertexCount + 1, CurrentVertexCount + 2,
			CurrentVertexCount + 2 , CurrentVertexCount + 1, CurrentVertexCount + 3});

	CurrentVertexCount += 4;
		

}

void Renderer2D::DrawCirlce(glm::vec2 pos, float radius, glm::vec4 color)
{

	if (CurrentVertexCount + 4 >= TotalVertices)
	{
		EndScene();
		BeginScene();
	}
			
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

void Renderer2D::DrawImage(uint32 t_Id, glm::vec2 pos, glm::vec2 size)
{
	if (CurrentVertexCount + 4 >= TotalVertices)
	{
		EndScene();
		BeginScene();
	}
		
	const auto& screenImage = ImageLib.Images[t_Id];

	auto slot = AttachTexture(screenImage.TexHandle);
	uint32 type = (slot << 8) | (3 << 0);
		
	CurrentVertex->pos = pos;
	CurrentVertex->type = type;
	CurrentVertex->additional.x = screenImage.ScreenPos.x;
	CurrentVertex->additional.y = screenImage.ScreenPos.y;
	++CurrentVertex;

	CurrentVertex->pos = glm::vec2{pos.x + size.x, pos.y};
	CurrentVertex->type = type;
	CurrentVertex->additional.x = screenImage.ScreenPos.x + screenImage.ScreenSize.x;
	CurrentVertex->additional.y = screenImage.ScreenPos.y;
	++CurrentVertex;

	CurrentVertex->pos = glm::vec2{pos.x, pos.y + size.y};
	CurrentVertex->type = type;
	CurrentVertex->additional.x = screenImage.ScreenPos.x;
	CurrentVertex->additional.y = screenImage.ScreenPos.y + screenImage.ScreenSize.y;
	++CurrentVertex;

	CurrentVertex->pos = pos + size;
	CurrentVertex->type = type;
	CurrentVertex->additional.x = screenImage.ScreenPos.x + screenImage.ScreenSize.x;
	CurrentVertex->additional.y = screenImage.ScreenPos.y + screenImage.ScreenSize.y;
	++CurrentVertex;


	Indices.insert(Indices.end(), { CurrentVertexCount , CurrentVertexCount + 1, CurrentVertexCount + 2,
			CurrentVertexCount + 2 , CurrentVertexCount + 1, CurrentVertexCount + 3});

	CurrentVertexCount += 4;
		
}

void Renderer2D::DrawText(std::string_view text, glm::vec2 pos, FontId typeface)
{
	if (CurrentVertexCount + 4 >= TotalVertices)
	{
		EndScene();
		BeginScene();
	}

	Memory::EstablishTempScope(Kilobytes(5));
	Defer { Memory::EndTempScope(); };
	TempVector<FontLibrary::AtlasEntry> entries;

	FontLib.GetEntries(typeface, text.data(), text.size(), entries);
		
	glm::vec2 currentPen{0.0f, 0.0f};
	for (const auto& entry : entries)
	{
		auto rect = pos + currentPen - entry.GlyphSize;
		currentPen += entry.Advance;

        // @Note: If the tex handle is 0, it meas that we
        // are handling the space "character" and hence we only
        // have and advance value
		if (entry.TexHandle == 0) continue;

		auto screenSize = glm::vec2{ (entry.Size.x) * FontLibrary::AtlasSize, (entry.Size.y) * FontLibrary::AtlasSize};

		auto slot = AttachTexture(entry.TexHandle);
		uint32 type = (slot << 8) | (5 << 0);

		CurrentVertex->pos = rect;
		CurrentVertex->color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
		CurrentVertex->uv = entry.Pos;
		CurrentVertex->type = type;
		++CurrentVertex;

		CurrentVertex->pos = rect + glm::vec2{screenSize.x, 0.0f};
		CurrentVertex->color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
		CurrentVertex->uv = entry.Pos + glm::vec2{entry.Size.x, 0.0f};
		CurrentVertex->type = type;
		++CurrentVertex;

		CurrentVertex->pos = rect + glm::vec2{0.0f, screenSize.y};
		CurrentVertex->uv = entry.Pos + glm::vec2{0.0f, entry.Size.y};
		CurrentVertex->color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
		CurrentVertex->type = type;
		++CurrentVertex;

		CurrentVertex->pos = rect + glm::vec2{screenSize.x, screenSize.y};
		CurrentVertex->color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
		CurrentVertex->uv = entry.Pos + glm::vec2{entry.Size.x, entry.Size.y};
		CurrentVertex->type = type;
		++CurrentVertex;

		Indices.insert(Indices.end(), { CurrentVertexCount , CurrentVertexCount + 1, CurrentVertexCount + 2,
				CurrentVertexCount + 2 , CurrentVertexCount + 1, CurrentVertexCount + 3});

		CurrentVertexCount += 4;
			
	}
}

void Renderer2D::DrawSubImage(uint32 t_Id, glm::vec2 pos, glm::vec2 size, glm::vec2 subPos, glm::vec2 subSize)
{
	if (CurrentVertexCount + 4 >= TotalVertices)
	{
		EndScene();
		BeginScene();
	}
		
	const auto& screenImage = ImageLib.Images[t_Id];

	auto slot = AttachTexture(screenImage.TexHandle);
	uint32 type = (slot << 8) | (3 << 0);
		
	CurrentVertex->pos = pos;
	CurrentVertex->type = type;
	CurrentVertex->additional.x = screenImage.ScreenPos.x + subPos.x / ImageAtlasSize;
	CurrentVertex->additional.y = screenImage.ScreenPos.y + subPos.y / ImageAtlasSize;
	++CurrentVertex;

	CurrentVertex->pos = glm::vec2{pos.x + size.x, pos.y};
	CurrentVertex->type = type;
	CurrentVertex->additional.x = screenImage.ScreenPos.x + subPos.x / ImageAtlasSize + subSize.x / ImageAtlasSize;
	CurrentVertex->additional.y = screenImage.ScreenPos.y + subPos.y / ImageAtlasSize;
	++CurrentVertex;

	CurrentVertex->pos = glm::vec2{pos.x, pos.y + size.y};
	CurrentVertex->type = type;
	CurrentVertex->additional.x = screenImage.ScreenPos.x + subPos.x / ImageAtlasSize;
	CurrentVertex->additional.y = screenImage.ScreenPos.y + subPos.y / ImageAtlasSize + subSize.y/ ImageAtlasSize;
	++CurrentVertex;

	CurrentVertex->pos = pos + size;
	CurrentVertex->type = type;
	CurrentVertex->additional.x = screenImage.ScreenPos.x + subPos.x / ImageAtlasSize + subSize.x / ImageAtlasSize;
	CurrentVertex->additional.y = screenImage.ScreenPos.y + subPos.y / ImageAtlasSize + subSize.y / ImageAtlasSize;
	++CurrentVertex;


	Indices.insert(Indices.end(), { CurrentVertexCount , CurrentVertexCount + 1, CurrentVertexCount + 2,
			CurrentVertexCount + 2 , CurrentVertexCount + 1, CurrentVertexCount + 3});

	CurrentVertexCount += 4;
		


}


// Drawing Lines

// Pusing transform matrices
