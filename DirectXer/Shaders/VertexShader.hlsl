
struct VSOut
{
    float4 pos : SV_Position;
    float3 world_pos : PositionOut;
    float2 uv : Texcoord;
    float3 color : COLOR0;
};

cbuffer CameraCbBuf : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
    uint shaderType;
};


VSOut main(float3 pos : Position, float3 color : Color, float2 uv: Texcoord)
{

    if (shaderType == 2)
    {
	VSOut vso;

	matrix view_m = view;
	
	view_m[3][0] = 0.0f;
	view_m[3][1] = 0.0f;
	view_m[3][2] = 0.0f;

	vso.pos = mul(float4(pos, 1.0), mul(model, mul(view_m, projection)));
	vso.world_pos = mul(float4(pos.x, pos.y, pos.z, 1.0), model);
	/* vso.world_pos = pos; */
	//vso.pos.z = vso.pos.w;
	
	return vso;
    }

    VSOut vso;
    vso.pos = mul(float4(pos.x, pos.y, pos.z, 1.0), mul(model, mul(view, projection)));
    vso.color = color;
    vso.uv = uv;
    

    
	
    return vso;
}
