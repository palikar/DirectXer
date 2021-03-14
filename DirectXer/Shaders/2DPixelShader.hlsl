struct PSIn
{
    float4 pos : SV_Position;
    float4 color : Color;
    float3 additional : Additional;
    uint type : Type;
};

cbuffer PSPrimBuf : register(b0)
{
    float3 CameraPos;
    uint shaderType;
};



SamplerState samp;
Texture2D textures[3] : register(t0);

float4 main(PSIn input) : SV_Target
{

    if ((0x0F & input.type) == 1 ) // Color Rect
    {
	return input.color;
    }
    else if((0x0F & input.type) == 2 )// Circle
    {
	float dist = sqrt(dot(input.additional.zy, input.additional.zy));
	if (dist > 1) discard;
	return input.color;
    }
    else if((0x0F & input.type) == 3 )// Image
    {
	uint slot = (0x00F0 & input.type) >> 8;
	float4 color4 = textures[slot].Sample(samp, input.additional.xy).rgba;
	float alpha = color4.a;
	float3 color = color4.rgb * color4.a;
	return float4(color, alpha);
    }

    return float4(1.0f, 0.0f, 1.0f, 1.0f);


}
