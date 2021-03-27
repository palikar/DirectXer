
struct VSOut
{
    float4 pos : SV_Position;
    float2 uv : Texcoord;
};

cbuffer VSPrim : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
    matrix invModel;
    uint shaderType;
};


VSOut main(uint vI : SV_VERTEXID)
{
    VSOut output = (VSOut)0;

    float2 texcoord = float2(vI & 1, vI >> 1);
    output.uv = texcoord;
    output.pos = float4((texcoord.x - 0.5f) * 2, -(texcoord.y - 0.5f) * 2, 0, 1);

    return output;
}
