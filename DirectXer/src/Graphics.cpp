
#include "Graphics.hpp"

void DrawFullscreenQuad(Graphics* Graphics, TextureId texture, ShaderConfig type)
{
	Graphics->BindTexture(1, texture);
	Graphics->SetShaderConfiguration(type);
	Graphics->UpdateCBs();
	Graphics->Draw(TT_TRIANGLE_STRIP, 4, 0);
}

TextureId NextTextureId()
{
	static TextureId next = 0;
	++next;
	return next;
}
