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
    float2 pos : Position;
    float2 uv: Texcoord;
    float4 color : Color;
    float3 additional : Additional;
    uint type : Type;
};


VSOut main(VSIn input)
{
    VSOut output = (VSOut)0;

    output.pos = mul(float4(input.pos.x, input.pos.y, 1.0f, 1.0f), projection);
    output.color = input.color;
    output.type = input.type;
    output.additional = input.additional;
    output.uv = input.uv;

    return output;
}
