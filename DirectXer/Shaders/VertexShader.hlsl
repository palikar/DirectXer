
struct VSOut
{
    float4 pos : SV_Position;
};

cbuffer CbBuf
{
    matrix transform;
};

VSOut main(float3 pos : Position)
{

    VSOut vso;
    vso.pos = mul(float4(pos.x, pos.y, pos.z, 1.0), transform);
	
    return vso;
}
