
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


VSOut main(uint vI : SV_VERTEXID, float3 pos : Position, float3 color : Color, float2 uv: Texcoord, float3 norm: Normal)
{
    VSOut output = (VSOut)0;
	
    if( shaderType == 7)
    {
	float2 texcoord = float2(vI & 1, vI >> 1);
	output.uv = texcoord;
	output.pos = float4((texcoord.x - 0.5f) * 2, -(texcoord.y - 0.5f) * 2, 0, 1);
	return output;
    }
    
    if (shaderType == 2)
    {
        matrix view_m = view;
	
	view_m[3][0] = 0.0f;
	view_m[3][1] = 0.0f;
	view_m[3][2] = 0.0f;

	output.pos = mul(float4(pos, 1.0), mul(model, mul(view_m, projection)));
	output.world_pos = (float3)mul(float4(pos.x, pos.y, pos.z, 1.0), model);	
	
	return output;
    }

    output.pos = mul(float4(pos.x, pos.y, pos.z, 1.0), mul(model, mul(view, projection)));
    output.color = color;
    output.world_pos = (float3)mul(float4(pos.x, pos.y, pos.z, 1.0), model);
    output.normal = mul((float3x3)(transpose(invModel)), norm);
    output.uv = uv;    
	
    return output;
}
