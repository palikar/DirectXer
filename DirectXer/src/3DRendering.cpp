
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

	// @Todo: Maybe move this into its own function
	DebugCBId = NextConstantBufferId();
	Gfx->CreateConstantBuffer(DebugCBId, sizeof(DebugCB), nullptr);
	Gfx->SetConstantBufferName(DebugCBId, "Debug Data CB");

	vbo = NextVertexBufferId();
	Gfx->CreateVertexBuffer(vbo, sizeof(Vertex3D), nullptr, (uint32)(sizeof(Vertex3D) * 1024), true);
	Gfx->SetVertexBufferName(vbo, "Render3D Main VB");
	
	ibo = NextIndexBufferId();
	Gfx->CreateIndexBuffer(ibo , nullptr, (uint32)(sizeof(uint32) * 1024 * 3), true);
	Gfx->SetIndexBufferName(ibo, "Render3D Main IB");
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

void Renderer3D::UpdateDebugData(float T)
{
	DebugCB debugBuffer;
	debugBuffer.DebugColor = std::abs(std::sin(T * 5))*float4{0.0f, 1.0f, 1.0f, 1.0f};
	Gfx->UpdateCBs(DebugCBId, sizeof(debugBuffer), &debugBuffer);
}

void Renderer3D::BindLighting()
{
	Gfx->BindPSConstantBuffers(LightingSetup.Cbo, 2);
	Gfx->BindVSConstantBuffers(LightingSetup.Cbo, 2);
}

void Renderer3D::SetupProjection(glm::mat4 matrix)
{
	CurrentProjection = (matrix);
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
}

void Renderer3D::DrawMeshWithMaterial(MeshId id, float3 pos, float3 scale)
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

void Renderer3D::DrawMesh(MeshId id, float3 pos, float3 scale)
{
	const auto mesh = MeshData.Meshes.at(id);
	Gfx->BindVertexBuffer(mesh.Geometry.Vbo);
	Gfx->BindIndexBuffer(mesh.Geometry.Ibo);

	Gfx->VertexShaderCB.model = init_translate(pos) * init_scale(scale);
	Gfx->VertexShaderCB.invModel = glm::inverse(Gfx->VertexShaderCB.model);
	Gfx->UpdateCBs();

	Gfx->DrawIndexed(TT_TRIANGLES, mesh.Geometry.Description.IndexCount, 0, 0);
}

void Renderer3D::DrawMesh(MeshId id, mat4 transform)
{
	const auto mesh = MeshData.Meshes.at(id);
	Gfx->BindVertexBuffer(mesh.Geometry.Vbo);
	Gfx->BindIndexBuffer(mesh.Geometry.Ibo);

	Gfx->VertexShaderCB.model = transform;
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

void Renderer3D::DrawDebugGeometry(uint32 id, float3 pos, float3 scale, glm::mat4 rotation)
{
	auto geom = DebugGeometries.Geometries[id];

	Gfx->BindVertexBuffer(DebugGeometries.Vbo, 0, 0);
	Gfx->BindIndexBuffer(DebugGeometries.Ibo);

	Gfx->VertexShaderCB.model = init_identity() * rotation * init_scale(scale) * init_translate(pos);
	Gfx->VertexShaderCB.invModel = glm::inverse(Gfx->VertexShaderCB.model);
	Gfx->UpdateCBs();

	Gfx->DrawIndexed(geom.Topology, geom.IndexCount, geom.IndexOffset, geom.BaseIndex);
}

void Renderer3D::DrawSelectedDebugGeometry(uint32 id, float3 pos, float3 scale, glm::mat4 rotation)
{
	auto geom = DebugGeometries.Geometries[id];

	Gfx->BindVertexBuffer(DebugGeometries.Vbo, 0, 0);
	Gfx->BindIndexBuffer(DebugGeometries.Ibo);

	Gfx->VertexShaderCB.model = init_identity() * rotation * init_translate(pos) * init_scale(scale);
	Gfx->VertexShaderCB.invModel = glm::inverse(Gfx->VertexShaderCB.model);

	Gfx->BindPSConstantBuffers(DebugCBId, 3);
	Gfx->SetShaderConfiguration(SC_COLOR);
	Gfx->SetRasterizationState(RS_DEBUG);
	Gfx->SetDepthStencilState(DSS_2DRendering);

	Gfx->UpdateCBs();
	
	Gfx->DrawIndexed(geom.Topology, geom.IndexCount, geom.IndexOffset, geom.BaseIndex);
	Gfx->SetRasterizationState(RS_NORMAL);
	Gfx->SetDepthStencilState(DSS_Normal);
}

void Renderer3D::DrawSelectedMesh(MeshId id, float3 pos, float3 scale)
{
	const auto mesh = MeshData.Meshes.at(id);
	Gfx->BindVertexBuffer(mesh.Geometry.Vbo);
	Gfx->BindIndexBuffer(mesh.Geometry.Ibo);

	Gfx->VertexShaderCB.model = init_translate(pos) * init_scale(scale);
	Gfx->VertexShaderCB.invModel = glm::inverse(Gfx->VertexShaderCB.model);

	Gfx->BindPSConstantBuffers(DebugCBId, 3);
	Gfx->SetShaderConfiguration(SC_COLOR);
	Gfx->SetRasterizationState(RS_DEBUG);
	Gfx->SetDepthStencilState(DSS_2DRendering);
	
	Gfx->UpdateCBs();
	
	Gfx->DrawIndexed(TT_TRIANGLES, mesh.Geometry.Description.IndexCount, 0, 0);

	Gfx->SetRasterizationState(RS_NORMAL);
	Gfx->SetDepthStencilState(DSS_Normal);
}

void Renderer3D::DrawSelectedMesh(MeshId id, mat4 transform)
{
	const auto mesh = MeshData.Meshes.at(id);
	Gfx->BindVertexBuffer(mesh.Geometry.Vbo);
	Gfx->BindIndexBuffer(mesh.Geometry.Ibo);

	Gfx->VertexShaderCB.model = transform;
	Gfx->VertexShaderCB.invModel = glm::inverse(Gfx->VertexShaderCB.model);

	Gfx->BindPSConstantBuffers(DebugCBId, 3);
	Gfx->SetShaderConfiguration(SC_COLOR);
	Gfx->SetRasterizationState(RS_DEBUG);
	Gfx->SetDepthStencilState(DSS_2DRendering);
	
	Gfx->UpdateCBs();
	
	Gfx->DrawIndexed(TT_TRIANGLES, mesh.Geometry.Description.IndexCount, 0, 0);

	Gfx->SetRasterizationState(RS_NORMAL);
	Gfx->SetDepthStencilState(DSS_Normal);
}

void Renderer3D::BeginScene(ShaderConfiguration config)
{
	CurrentConfig = config;
	Gfx->SetShaderConfiguration(config);
}

void Renderer3D::EndScene()
{
}

void Renderer3D::BeginLines()
{
	Indices.clear();
	Vertices.clear();
	Vertices.resize(1024);

	CurrentVertexCount = 0;
	CurrentVertex = &Vertices[0];
}

void Renderer3D::DrawLine(float3 from, float3 to, float4 color)
{
	CurrentVertex->pos = float4{ from , 1.0f};
	CurrentVertex->color = color;
	++CurrentVertex;

	CurrentVertex->pos = float4{ to , 1.0f };
	CurrentVertex->color = color;
	++CurrentVertex;

	Indices.insert(Indices.end(), { CurrentVertexCount , CurrentVertexCount + 1 });

	CurrentVertexCount += 2;
}

void Renderer3D::EndLines()
{
	UpdateCamera();
	Gfx->VertexShaderCB.model = glm::mat4(1.0f);
	Gfx->SetShaderConfiguration(SC_3D_LINES);
	Gfx->BindIndexBuffer(ibo);
	Gfx->BindVertexBuffer(vbo, 0, 0);

	Gfx->UpdateVertexBuffer(vbo, Vertices.data(), CurrentVertexCount * sizeof(Vertex3D));
	Gfx->UpdateIndexBuffer(ibo, Indices.data(), 3u * CurrentVertexCount * sizeof(uint32));
	Gfx->UpdateCBs();
	Gfx->DrawIndexed(TT_LINES, (uint32)Indices.size(), 0u, 0u);
}
