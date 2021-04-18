#pragma once

#ifdef _WIN32

#include <PlatformWindows/GraphicsD3D11.hpp>
using Graphics = GraphicsD3D11;

#endif

template<typename VertexType>
VertexBufferId vertexBufferFactory(VertexBufferId id, Graphics& graphics, TempVector<VertexType>& t_VertexList)
{
	graphics.CreateVertexBuffer(id, sizeof(VertexType), t_VertexList.data(), (uint32)(sizeof(VertexType) * t_VertexList.size()));
	return id;
}

template<typename IndexType = uint32>
IndexBufferId indexBufferFactory(IndexBufferId id, Graphics& graphics, TempVector<IndexType>& t_IndexList)
{
	graphics.CreateIndexBuffer(id, t_IndexList.data(), (uint32)(sizeof(IndexType) * t_IndexList.size()));
	return id;
}

void DrawFullscreenQuad(Graphics* Graphics, TextureObject texture, ShaderConfig type);

TextureId NextTextureId();
ConstantBufferId NextConstantBufferId();
IndexBufferId NextIndexBufferId();
VertexBufferId NextVertexBufferId();
