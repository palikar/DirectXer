#include "GraphicsOpenGL.hpp"


static GLenum TFToGL(TextureFormat format)
{
	switch (format)
	{
	  case TF_RGBA: return GL_RGBA8;
	  case TF_A: return GL_A8;
	  case TF_R: return GL_R8;
	}

	Assert(false, "Usage of unknows texture format.");
	return GL_R8;
}


void GraphicsOpenGL::InitSwapChain(SwapChainSettings settings)
{
	Display x11Display = (Display) settings.Display;
}

void GraphicsOpenGL::InitBackBuffer()
{

}

void GraphicsOpenGL::InitZBuffer(float, float)
{

}

void GraphicsOpenGL::InitResources()
{

}

void GraphicsOpenGL::InitRasterizationsStates()
{

}

void GraphicsOpenGL::InitSamplers()
{

}

void GraphicsOpenGL::InitBlending()
{

}

void GraphicsOpenGL::InitDepthStencilStates()
{

}

void GraphicsOpenGL::ResizeBackBuffer(float width, float height)
{

}

void GraphicsOpenGL::BindTexture(uint32 t_Slot, TextureId t_Id)
{

}

void GraphicsOpenGL::BindPSConstantBuffers(ConstantBufferId t_Id, uint16 t_Slot)
{

}

void GraphicsOpenGL::BindVSConstantBuffers(ConstantBufferId t_Id, uint16 t_Slot)
{

}

void GraphicsOpenGL::BindVertexBuffer(VertexBufferId t_Id, uint32 offset = 0)
{

}

void GraphicsOpenGL::BindIndexBuffer(IndexBufferId id)
{

}

void GraphicsOpenGL::SetScissor(Rectangle2D t_Rect)
{
	glScissor((GLint)t_Rect.Position.x, (GLint)t_Rect.Position.y,
			  (GLsizei)t_Rect.Size.x, (GLsizei)t_Rect.Size.y);
}

void GraphicsOpenGL::SetRasterizationState(RasterizationState t_State = RS_DEBUG)
{

}

void GraphicsOpenGL::SetDepthStencilState(DepthStencilState t_State = DSS_Normal, uint32 t_RefValue = 0)
{

}

void GraphicsOpenGL::SetViewport(float x, float y, float width, float height)
{
	glViewport((GLint)x, (GLint)y, (GLsizei)width, (GLsizei)height);
}

void GraphicsOpenGL::SetShaderConfiguration(ShaderConfig t_Confing)
{

}

void GraphicsOpenGL::SetBlendingState(BlendingState t_State)
{

}

void GraphicsOpenGL::SetRenderTarget(RTObject& t_RT)
{

}

void GraphicsOpenGL::ResetRenderTarget()
{

}

bool GraphicsOpenGL::CreateTexture(TextureId id, TextureDescription description, const void* t_Data)
{

}

bool GraphicsOpenGL::CreateDSTexture(TextureId id, TextureDescription description)
{

}

bool GraphicsOpenGL::CreateRenderTexture(TextureId t_Id, RenderTargetDescription description)
{

}

bool GraphicsOpenGL::CreateCubeTexture(TextureId id, TextureDescription description, void* t_Data[6])
{

}

bool GraphicsOpenGL::CreateVertexBuffer(VertexBufferId id, uint32 structSize, void* data, uint32 dataSize, bool dynamic = false)
{

}

bool GraphicsOpenGL::CreateIndexBuffer(IndexBufferId id, void* data, uint32 dataSize, bool dynamic = false)
{

}

bool GraphicsOpenGL::CreateConstantBuffer(ConstantBufferId id, uint32 t_Size, void* t_InitData)
{

}

void GraphicsOpenGL::UpdateCBs()
{

}

void GraphicsOpenGL::UpdateCBs(ConstantBufferId& t_Id, uint32 t_Length, void* t_Data)
{

}

void GraphicsOpenGL::UpdateVertexBuffer(VertexBufferId t_Id, void* data, uint64 t_Length)
{

}

void GraphicsOpenGL::UpdateIndexBuffer(IndexBufferId t_Id, void* data, uint64 t_Length)
{

}

void GraphicsOpenGL::UpdateTexture(TextureId t_Id, Rectangle2D rect, const void* t_Data, int t_Pitch = 4)
{

}

void GraphicsOpenGL::DrawIndex(TopolgyType topology, uint32 count, uint32 offset = 0,  uint32 base = 0)
{

}

void GraphicsOpenGL::Draw(TopolgyType topology, uint32 count, uint32 base)
{

}

void GraphicsOpenGL::ClearBuffer(float red, float green, float blue)
{

}

void GraphicsOpenGL::ClearZBuffer()
{

}

void GraphicsOpenGL::ClearRT(RTObject& t_RT)
{

}

void GraphicsOpenGL::EndFrame()
{

}

void GraphicsOpenGL::DestroyZBuffer()
{

}

void GraphicsOpenGL::Destroy()
{

}

