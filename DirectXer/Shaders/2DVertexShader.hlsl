struct VSOut
{
    float4 pos : SV_Position;
};

VSOut main(float2 pos : Position, float2 pos : Texcoord, float3 color : Color)
{
    VSOut vsout = (VSOut)0;

    return vsout;
}
