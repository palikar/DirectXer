struct PSIn
{
    float4 pos : SV_Position;
    float3 world_pos : PositionOut;
    float2 uv : Texcoord;
    float3 color : COLOR0;
    float3 normal: Normal;
};

cbuffer PSPrimBuf : register(b0)
{
    float3 CameraPos;
    uint shaderType;
};

cbuffer DebugCB : register(b3)
{
    float4 DebugColor;
}
    
SamplerState samp;

TextureCube env : register(t0);
Texture2D tex_1 : register(t1);
Texture2D tex_2 : register(t2);

float4 main(PSIn input) : SV_Target
{
    if(shaderType == 1) // Color
    {
	return float4(input.color, 1.0f);
    }
    else if(shaderType == 2) // Skybox
    {
	return env.Sample(samp, input.world_pos);
    }
    else if(shaderType == 3) // Simple texture
    {
	return tex_1.Sample(samp, input.uv);
    }
    else if(shaderType == 10) // Debug color
    {
	return DebugColor;
    }

    return float4(1.0f, 0.0f, 0.5f, 1.0f);
}
