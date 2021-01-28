

struct PSIn
{
    float4 pos : SV_Position;
    float2 uv : Texcoord;
    float3 color : COLOR0;
};

cbuffer CbBuf
{
    float4 color;
    uint shaderType;
};

SamplerState samp;

Texture2D tex;

float4 main(PSIn input) : SV_Target
{
    if (shaderType == 0) // Tex
    {
	return tex.Sample(samp, input.uv);
    }
    else if(shaderType == 1) // Color
    {
	return float4(input.color, 1.0f);
    }

    return float4(1.0f, 0.0f, 0.5f, 1.0f);
}
