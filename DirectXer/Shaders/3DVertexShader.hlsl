struct VSOut
{
    float4 pos : SV_Position;
    float4 color : Color;
    float2 uv : Texcoord;
    float3 additional : Additional;
    uint type : Type;
};

cbuffer VSPrim : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
    matrix invModel;
    uint shaderType;
};

struct VSIn
{
    float4 pos : Position;
    float4 color : Color;
};

VSOut main(VSIn input)
{
    VSOut output = (VSOut)0;

    output.pos = mul(float4(input.pos.x, input.pos.y, input.pos.z, 1.0), mul(model, mul(view, projection)));
    output.color = input.color;

    return output;
}
