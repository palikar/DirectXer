#pragma once

#ifdef _WIN32

#include <PlatformWindows/GraphicsD3D11.hpp>
using Graphics = GraphicsD3D11;

#endif



template<typename VertexType>
VBObject vertexBufferFactory(Graphics& graphics, TempVector<VertexType>& t_VertexList)
{
	return graphics.CreateVertexBuffer(sizeof(VertexType), t_VertexList.data(), (uint32)(sizeof(VertexType) * t_VertexList.size()));
}

template<typename IndexType = uint32>
IBObject indexBufferFactory(Graphics& graphics, TempVector<IndexType>& t_IndexList)
{
	return graphics.CreateIndexBuffer(t_IndexList.data(), (uint32)(sizeof(IndexType) * t_IndexList.size()));
}

void DrawFullscreenQuad(Graphics* Graphics, TextureObject texture, ShaderConfig type);