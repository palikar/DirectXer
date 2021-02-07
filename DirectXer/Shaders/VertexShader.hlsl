
struct VSOut
{
    float4 pos : SV_Position;
    float3 world_pos : PositionOut;
    float2 uv : Texcoord;
    float3 color : COLOR0;
    float3 normal: Normal;
};

cbuffer VSPrim : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
    matrix invModel;
    uint shaderType;
};


VSOut main(float3 pos : Position, float3 color : Color, float2 uv: Texcoord, float3 norm: Normal)
{

    if (shaderType == 2)
    {
        VSOut vso = (VSOut)0;

	matrix view_m = view;
	
	view_m[3][0] = 0.0f;
	view_m[3][1] = 0.0f;
	view_m[3][2] = 0.0f;

	vso.pos = mul(float4(pos, 1.0), mul(model, mul(view_m, projection)));
	vso.world_pos = (float3)mul(float4(pos.x, pos.y, pos.z, 1.0), model);
	
	
	return vso;
    }

    VSOut output = (VSOut)0;
    output.pos = mul(float4(pos.x, pos.y, pos.z, 1.0), mul(model, mul(view, projection)));
    output.color = color;
    output.world_pos = (float3)mul(float4(pos.x, pos.y, pos.z, 1.0), model);
    output.normal = mul((float3x3)(transpose(invModel)), norm);
    output.uv = uv;    
	
    return output;
}
