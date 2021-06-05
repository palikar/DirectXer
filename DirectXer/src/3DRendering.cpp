
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
	MeshData.Materials.Init();
}

void Renderer3D::InitLighting()
{
	LightingSetup.Cbo = NextConstantBufferId();
	Gfx->CreateConstantBuffer(LightingSetup.Cbo, sizeof(Lighting), &LightingSetup.LightingData);
	Gfx->SetConstantBufferName(LightingSetup.Cbo, "Lighting CB");
}

void Renderer3D::InitInstancedDataBuffer(uint32 maxInstances)
{
	InstancedData.resize(maxInstances);
	InstancedBuffer = NextVertexBufferId();
	Gfx->CreateVertexBuffer(InstancedBuffer, sizeof(MtlInstanceData), nullptr, maxInstances * sizeof(MtlInstanceData), true);
	Gfx->SetVertexBufferName(InstancedBuffer, "MTLInstancedData");
}

void Renderer3D::UpdateInstancedData()
{
	Gfx->UpdateVertexBuffer(InstancedBuffer, InstancedData.data(), InstancedData.size() * sizeof(MtlInstanceData));
}

void Renderer3D::UpdateCamera()
{
	Gfx->VertexShaderCB.view = glm::transpose(CurrentCamera.view());
	Gfx->PixelShaderCB.cameraPos = CurrentCamera.Pos;
	Gfx->VertexShaderCB.cameraPos = CurrentCamera.Pos;
}

void Renderer3D::UpdateLighting()
{
	Gfx->UpdateCBs(LightingSetup.Cbo, sizeof(Lighting), &LightingSetup.LightingData);
}

void Renderer3D::BindLighting()
{
	Gfx->BindPSConstantBuffers(LightingSetup.Cbo, 2);
	Gfx->BindVSConstantBuffers(LightingSetup.Cbo, 2);
}

void Renderer3D::SetupProjection(glm::mat4 matrix)
{
	Gfx->VertexShaderCB.projection = glm::transpose(matrix);
}

void Renderer3D::BindMaterial(MaterialId id)
{
	MeshData.Materials.Bind(Gfx, id);
}

void Renderer3D::BindMaterialInstanced(MaterialId id)
{
	MeshData.Materials.BindInstanced(Gfx, id);
}

void Renderer3D::DrawSkyBox(TextureId sky)
{
	Gfx->SetRasterizationState(RS_BACK);
	Gfx->SetShaderConfiguration(SC_DEBUG_SKY);
	Gfx->BindIndexBuffer(DebugGeometries.Ibo);
	Gfx->BindVertexBuffer(DebugGeometries.Vbo);
	Gfx->BindTexture(0, sky);
	Gfx->VertexShaderCB.model = init_scale(500.0f, 500.0f, 500.0f) * init_translate(0.0f, 0.0f, 0.0f);
	Gfx->VertexShaderCB.invModel = glm::inverse(Gfx->VertexShaderCB.model);
	Gfx->UpdateCBs();

	auto geom = DebugGeometries.Geometries[0];
	Gfx->DrawIndexed(geom.Topology, geom.IndexCount, geom.BaseIndex, geom.IndexOffset);
	Gfx->SetRasterizationState(RS_NORMAL);
}

void Renderer3D::DrawMeshWithMaterial(MeshId id, glm::vec3 pos, glm::vec3 scale)
{
	const auto mesh = MeshData.Meshes.at(id);
	MeshData.Materials.Bind(Gfx, mesh.Material);

	Gfx->BindVertexBuffer(mesh.Geometry.Vbo);
	Gfx->BindIndexBuffer(mesh.Geometry.Ibo);

	Gfx->VertexShaderCB.model = init_translate(pos) * init_scale(scale);
	Gfx->VertexShaderCB.invModel = glm::inverse(Gfx->VertexShaderCB.model);
	Gfx->UpdateCBs();

	Gfx->DrawIndexed(TT_TRIANGLES, mesh.Geometry.Description.IndexCount, 0, 0);
}

void Renderer3D::DrawMesh(MeshId id, glm::vec3 pos, glm::vec3 scale)
{
	const auto mesh = MeshData.Meshes.at(id);
	Gfx->BindVertexBuffer(mesh.Geometry.Vbo);
	Gfx->BindIndexBuffer(mesh.Geometry.Ibo);

	Gfx->VertexShaderCB.model = init_translate(pos) * init_scale(scale);
	Gfx->VertexShaderCB.invModel = glm::inverse(Gfx->VertexShaderCB.model);
	Gfx->UpdateCBs();

	Gfx->DrawIndexed(TT_TRIANGLES, mesh.Geometry.Description.IndexCount, 0, 0);
}

void Renderer3D::DrawInstancedMesh(MeshId id, uint32 instancesCount, uint32 baseInstanced)
{
	const auto mesh = MeshData.Meshes.at(id);
	Gfx->BindVertexBuffer(mesh.Geometry.Vbo);
	Gfx->BindIndexBuffer(mesh.Geometry.Ibo);

	Gfx->BindVertexBuffer(InstancedBuffer, 0, 1);

	Gfx->UpdateCBs();

	Gfx->DrawInstancedIndex(TT_TRIANGLES, mesh.Geometry.Description.IndexCount, instancesCount, 0, 0, baseInstanced);
}

void Renderer3D::DrawDebugGeometry(uint32 id, glm::vec3 pos, glm::vec3 scale, glm::mat4 rotation)
{
	auto geom = DebugGeometries.Geometries[id];

	Gfx->BindVertexBuffer(DebugGeometries.Vbo, 0, 0);
	Gfx->BindIndexBuffer(DebugGeometries.Ibo);

	Gfx->VertexShaderCB.model = init_identity() * rotation * init_translate(pos) * init_scale(scale);
	Gfx->VertexShaderCB.invModel = glm::inverse(Gfx->VertexShaderCB.model);
	Gfx->UpdateCBs();

	Gfx->DrawIndexed(geom.Topology, geom.IndexCount, geom.IndexOffset, geom.BaseIndex);
}

void Renderer3D::BeginScene(ShaderConfiguration config)
{
	CurrentConfig = config;
	Gfx->SetShaderConfiguration(config);
}

void Renderer3D::EndScene()
{

}
