

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
    
    /* return float4(input.color, 1.0f); */
    return tex.Sample(samp, input.uv);
    
}
