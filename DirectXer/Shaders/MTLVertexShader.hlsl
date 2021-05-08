
struct PointLight
{
    float4 Color;
    float4 Position;
    float4 Params;
    bool active;
};

struct SpotLight
{
    float4 Color;
    float4 Position;
    float4 Dir;
    float4 Params;
    bool active;
};


struct VSIn
{
    float3 pos : Position;
    float2 uv : Texcoord;
    float3 normal : Normal;
};

struct VSOut
{
    float4 pos : SV_Position;
    float3 color : COLOR0;
};


cbuffer VSPrim : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
    matrix invModel;
    uint shaderType;
};

cbuffer MTLMat : register(b1)
{
    float Ns;
    float3 Ka;

    float Ni;
    float3 Kd;

    float d;
    float3 Ks;

    float3 Ke;
    uint illum;
};

cbuffer LightningBuf : register(b2)
{
    float4 AmbLightColor;

    float4 DirLightColor;
    float4 DirLightDir;

    PointLight PointLights[5];
    SpotLight SpotLights[5];
    
};


SamplerState linearSamp : register(s0);
SamplerState pointSamp : register(s1);

Texture2D KaMap : register(t0);
Texture2D KdMap : register(t1);
Texture2D KsMap : register(t3);
Texture2D NsMap : register(t4);
Texture2D dMap  : register(t5);


VSOut main( VSIn input)
{
    VSOut output = (VSOut)0;

    output.pos = mul(float4(input.pos.x, input.pos.y, input.pos.z, 1.0), mul(model, mul(view, projection)));

    output.color = KdMap.SampleLevel(pointSamp, input.uv, 0).rgb;
	

    return output;
}
