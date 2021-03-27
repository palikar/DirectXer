
#include "Graphics.hpp"

void DrawFullscreenQuad(Graphics* Graphics, TextureObject texture, ShaderConfig type)
{
	Graphics->BindTexture(1, texture);
	Graphics->SetShaderConfiguration(type);
	Graphics->UpdateCBs();
	Graphics->Draw(TT_TRIANGLE_STRIP, 4, 0);
}
