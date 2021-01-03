
struct VSOut
{
    float4 pos : SV_Position;
};

cbuffer CbBuf
{
    matrix model;
    matrix view;
    matrix projection;
};

VSOut main(float3 pos : Position)
{

    VSOut vso;
    vso.pos = mul(float4(pos.x, pos.y, pos.z, 1.0), mul(model, mul(view, projection)));
	
    return vso;
}
