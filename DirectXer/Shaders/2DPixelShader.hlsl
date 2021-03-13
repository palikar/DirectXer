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
Texture2D tex_1 : register(t0);

float4 main(PSIn input) : SV_Target
{

    if (input.type == 1 ) // Color Rect
    {
	return input.color;
    }
    else if(input.type == 2 )// Circle
    {
	float dist = sqrt(dot(input.additional.zy, input.additional.zy));
	if (dist > 1) discard;	
        return input.color;
    }
    else if(input.type == 3 )// Image
    {
	return tex_1.Sample(samp, input.additional.xy).rgba;
    }

    return float4(1.0f, 0.0f, 1.0f, 1.0f);

    
}
