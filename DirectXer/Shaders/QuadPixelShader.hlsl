struct PSIn
{
    float4 pos : SV_Position;
    float2 uv : Texcoord;
};

cbuffer PSPrimBuf : register(b0)
{
    float3 CameraPos;
    uint shaderType;
};

SamplerState samp;
Texture2D tex_1 : register(t1);

float4 main(PSIn input) : SV_Target
{

    if (shaderType == 7) // Phong
    {
        return tex_1.Sample(samp, input.uv);
    }

    return float4(1.0f, 0.0f, 1.0f, 1.0f);

}
