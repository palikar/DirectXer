
#include "3DRendering.hpp"

#include <GeometryUtils.hpp>


void Renderer3D::InitDebugGeometry(DebugGeometryBuilder& builder)
{
	IndexedGPUBuffer buffers = builder.CreateBuffer(Gfx);

	DebugGeometries.Ibo = buffers.ibo;
	DebugGeometries.Vbo = buffers.vbo;

	DebugGeometries.Geometries.resize(builder.GeometryBuffer.Geometries.size());
	
	memcpy(DebugGeometries.Geometries.data(), builder.GeometryBuffer.Geometries.data(),
		   builder.GeometryBuffer.Geometries.size() * sizeof(GPUGeometryInfo));
}

void Renderer3D::InitRenderer(Graphics* t_Graphics)
{
	Gfx = t_Graphics;
	MeshData.Meshes.reserve(32); 
	MeshData.Materials.reserve(32); 
}

void Renderer3D::InitLighting()
{
	Lighting.Cbo = NextConstantBufferId();
	Gfx->CreateConstantBuffer(Lighting.Cbo, sizeof(Lighting), &Lighting.Lighting);
	Gfx->SetConstantBufferName(Lighting.Cbo, "Lighting CB");
}

void Renderer3D::SetupCamera(Camera t_Camera)
{
	Gfx->VertexShaderCB.view = glm::transpose(t_Camera.view());
	Gfx->PixelShaderCB.cameraPos = t_Camera.Pos;
	Gfx->VertexShaderCB.cameraPos = t_Camera.Pos;
}

void Renderer3D::SetupProjection(glm::mat4 matrix)
{
	Gfx->VertexShaderCB.projection = glm::transpose(matrix);
}

void Renderer3D::DrawSkyBox(TextureId sky)
{
	Gfx->BindIndexBuffer(DebugGeometries.Ibo);
	Gfx->BindVertexBuffer(DebugGeometries.Vbo);
	Gfx->SetShaderConfiguration(SC_DEBUG_SKY);
	Gfx->BindTexture(0, sky);
	Gfx->VertexShaderCB.model = init_scale(500.0f, 500.0f, 500.0f) * init_translate(0.0f, 0.0f, 0.0f);
	Gfx->VertexShaderCB.invModel = glm::inverse(Gfx->VertexShaderCB.model);
	Gfx->UpdateCBs();

	auto geom = DebugGeometries.Geometries[0];
	Gfx->DrawIndexed(geom.Topology, geom.IndexCount, geom.BaseIndex, geom.IndexOffset);
}

void Renderer3D::DrawMesh(MeshId id, glm::vec3 pos, glm::vec3 scale)
{
	const auto mesh = MeshData.Meshes.at(id);
	const auto material = MeshData.Materials.at(mesh.Material);

	Gfx->SetShaderConfiguration(material.Program);

	Gfx->BindVSConstantBuffers(material.Cbo, 1);
		
	Gfx->BindVertexBuffer(mesh.Geometry.Vbo);
	Gfx->BindIndexBuffer(mesh.Geometry.Ibo);
		
	Gfx->VertexShaderCB.model = init_translate(pos) * init_scale(scale);
	Gfx->VertexShaderCB.invModel = glm::inverse(Gfx->VertexShaderCB.model);
	Gfx->UpdateCBs();

	if (material.KaMap) Gfx->BindVSTexture(0, material.KaMap);
	if (material.KdMap) Gfx->BindVSTexture(1, material.KdMap);

	Gfx->DrawIndexed(TT_TRIANGLES, mesh.Geometry.Description.IndexCount, 0, 0);
}

void Renderer3D::DrawDebugGeometry(uint32 id, glm::vec3 pos, glm::vec3 scale, glm::mat4 rotation)
{
	auto geom = DebugGeometries.Geometries[id];

	Gfx->BindVertexBuffer(DebugGeometries.Vbo, 0, 0);
	Gfx->BindIndexBuffer(DebugGeometries.Ibo);
	
	Gfx->VertexShaderCB.model = rotation * init_translate(pos) * init_scale(scale);
	Gfx->VertexShaderCB.invModel = glm::inverse(Gfx->VertexShaderCB.model);
	Gfx->UpdateCBs();

	Gfx->DrawIndexed(geom.Topology, geom.IndexCount, geom.BaseIndex, geom.IndexOffset);
	
}

void Renderer3D::BeginScene(ShaderConfig config)
{
	CurrentConfig = config;
	Gfx->SetShaderConfig(config);
}

void Renderer3D::EndScene()
{

}
