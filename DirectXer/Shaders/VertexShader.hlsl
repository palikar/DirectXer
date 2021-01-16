
struct VSOut
{
    float4 pos : SV_Position;
    float3 color : COLOR0;
};

cbuffer CbBuf
{
    matrix model;
    matrix view;
    matrix projection;
};

VSOut main(float3 pos : Position, float3 color : Color)
{

    VSOut vso;
    vso.pos = mul(float4(pos.x, pos.y, pos.z, 1.0), mul(model, mul(view, projection)));
    vso.color = color;
	
    return vso;
}
