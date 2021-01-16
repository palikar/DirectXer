

struct PSIn
{
    float4 pos : SV_Position;
    float3 color : COLOR0;
};

cbuffer CbBuf
{
	float4 color;
};

float4 main(PSIn input) : SV_Target
{
    
    return float4(input.color, 1.0f);
    
}
