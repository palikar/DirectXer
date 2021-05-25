struct VSOut
{
    float4 pos : SV_Position;
    float3 world_pos : PositionOut;
    float2 uv : Texcoord;
    float3 normal: Normal;
};

cbuffer VSPrim : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
    matrix invModel;
    uint shaderType;
    float3 CameraPos;
};

VSOut main(float3 pos : Position, float2 uv: Texcoord, float3 norm: Normal)
{
    VSOut output = (VSOut)0;
	
    output.pos = mul(float4(pos.x, pos.y, pos.z, 1.0), mul(model, mul(view, projection)));
    output.world_pos = (float3)mul(float4(pos.x, pos.y, pos.z, 1.0), model);
    output.normal = mul((float3x3)(transpose(invModel)), norm);
    output.uv = uv;    
	
    return output;
}
