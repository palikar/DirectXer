#pragma once

#include <Logging.hpp>
#include <Memory.hpp>
#include <Math.hpp>
#include <Utils.hpp>
#include <GraphicsContainers.hpp>

#include "IncludeLinux.hpp"
#include <GL/glew.h>

struct VBObject
{
	GLuint vbo;
};

struct IBObject
{
	GLuint vio;
};

struct CBObject
{
	GLuint cbo;
};

struct TextureObject
{
	uint32 texture;
};

struct ShaderObject
{
	GLuint shader;
};

struct BlendingSetup
{
	bool Enabled;
	
	GLenum BlenidngOp;
	GLenum BlenidngOpAlpha;

	GLenum BlenidngSource;
	GLenum BlenidngSourceAlpha;

	GLenum BlenidngDest;
	GLenum BlenidngDestAlpha;	
};

struct DepthStencilSetup
{
	bool DepthEnabled;
	GLenum DepthFunc;
	GLuint DepthWriteMask;

	bool StencilEnabled;
	GLuint StencilWriteMask;
	GLuint StencilReadMask;

	GLenum StencilFrontFailOp;
	GLenum StencilFrontFunc;
	GLenum StencilFrontPassOp;
	GLenum StencilFrontDepthFailOp;

	GLenum StencilBackFailOp;
	GLenum StencilBackFunc;
	GLenum StencilBackPassOp;
	GLenum StencilBackDepthFailOp;
};

struct RasterizationSetup
{
	GLenum CullMode;
	GLenum FillMode;
	GLenum Clockwise;
	bool ScissorEnable;
};

class GraphicsOpenGL
{
  public:

    using VertexBufferType = VBObject;
    using IndexBufferType = IBObject;
    using ConstantBufferType = CBObject;
    using TextureType = TextureObject;
    using ShaderType = ShaderObject;

    void InitSwapChain(SwapChainSettings settings);
    void InitBackBuffer();
    void InitZBuffer(float width, float height);
    void InitResources();
    void InitRasterizationsStates();
    void InitSamplers();
    void InitBlending();
    void InitDepthStencilStates();
	
    void ResizeBackBuffer(float width, float height);

    void BindTexture(uint32 t_Slot, TextureId t_Id);
    void BindPSConstantBuffers(ConstantBufferId t_Id, uint16 t_Slot);
    void BindVSConstantBuffers(ConstantBufferId t_Id, uint16 t_Slot);
    void BindVertexBuffer(VertexBufferId t_Id, uint32 offset = 0);
    void BindIndexBuffer(IndexBufferId id);

    void SetScissor(Rectangle2D t_Rect);
    void SetRasterizationState(RasterizationState t_State = RS_DEBUG);
    void SetDepthStencilState(DepthStencilState t_State = DSS_Normal, uint32 t_RefValue = 0);
    void SetViewport(float x, float y, float width, float height);
    void SetShaderConfiguration(ShaderConfig t_Confing);
    void SetBlendingState(BlendingState t_State);
    void SetRenderTarget(RTObject& t_RT);
    void ResetRenderTarget();

    bool CreateTexture(TextureId id, TextureDescription description, const void* t_Data);
    bool CreateDSTexture(TextureId id, TextureDescription description);
    bool CreateRenderTexture(TextureId t_Id, RenderTargetDescription description);
    bool CreateCubeTexture(TextureId id, TextureDescription description, void* t_Data[6]);
    bool CreateVertexBuffer(VertexBufferId id, uint32 structSize, void* data, uint32 dataSize, bool dynamic = false);
    bool CreateIndexBuffer(IndexBufferId id, void* data, uint32 dataSize, bool dynamic = false);
	bool CreateConstantBuffer(ConstantBufferId id, uint32 t_Size, void* t_InitData);

	void UpdateCBs();
	void UpdateCBs(ConstantBufferId& t_Id, uint32 t_Length, void* t_Data);
	void UpdateVertexBuffer(VertexBufferId t_Id, void* data, uint64 t_Length);
	void UpdateIndexBuffer(IndexBufferId t_Id, void* data, uint64 t_Length);
	void UpdateTexture(TextureId t_Id, Rectangle2D rect, const void* t_Data, int t_Pitch = 4);

	void DrawIndex(TopolgyType topology, uint32 count, uint32 offset = 0,  uint32 base = 0);
	void Draw(TopolgyType topology, uint32 count, uint32 base);

	void ClearBuffer(float red, float green, float blue);
	void ClearZBuffer();
	void ClearRT(RTObject& t_RT);
	void EndFrame();

	void DestroyZBuffer();
	void Destroy();

  public:

	RasterizationSetup RasterizationsStates[RS_COUNT];
	BlendingSetup BlendingStates[BS_Count];
	DepthStencilSetup DepthStencilStates[DSS_Count];
	ShaderObject Shaders[SF_COUNT];

	GLint PixelShaderCBId;
	GLint VertexShaderCBId;
	PSConstantBuffer PixelShaderCB;
	VSConstantBuffer VertexShaderCB;

	GPUResourceMap<TextureId, TextureObject, GPURes_Texture> Textures;
	GPUResourceMap<VertexBufferId, VBObject, GPURes_VertexBuffer> VertexBuffers;
	GPUResourceMap<IndexBufferId, IBObject, GPURes_IndexBuffer> IndexBuffers;
	GPUResourceMap<ConstantBufferId, CBObject, GPURes_ConstantBuffer> ConstantBuffers;
};
