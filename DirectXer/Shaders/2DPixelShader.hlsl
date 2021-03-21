struct PSIn
{
    float4 pos : SV_Position;
    float4 color : Color;
    float2 uv : Texcoord;
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
    if ((0xFF & input.type) == 1 ) // Color Rect
    {
	return input.color;
    }
    else if((0xFF & input.type) == 2 )// Circle
    {
	float dist = sqrt(dot(input.additional.zy, input.additional.zy));
	if (dist > 1) discard;
	return input.color;
    }
    else if((0xFF & input.type) == 3 )// Image
    {
	int slot = (0xFF00 & input.type) >> 8;
	float4 color4 = (float4)0;
	if (slot == 0) color4 = textures[0].Sample(samp, input.additional.xy).rgba;
	if (slot == 1) color4 = textures[1].Sample(samp, input.additional.xy).rgba;
	if (slot == 2) color4 = textures[2].Sample(samp, input.additional.xy).rgba;
	
	float alpha = color4.a;
	float3 color = color4.rgb * color4.a;
	return float4(color, alpha);
    }
    else if((0xFF & input.type) == 4 )// Rounded rect
    {
	float2 pos = (input.additional.yz +1.0f)/2.0f;
	float r = (input.additional.x + 1.0f)/2.0f;
	if( (pos.x < r) && (pos.y < r)
	    || (pos.x < r) && (pos.y > 1.0f - r)
	    || (pos.x > 1.0f - r) && (pos.y < r)
	    || (pos.x > 1.0f - r) && (pos.y > 1.0f - r)
	    )
	{
	    float2 p = float2(pos.x, pos.y);
	    if(pos.y > 1.0f - r) p.y = 1 - p.y;
	    if(pos.x > 1.0f - r) p.x = 1 - p.x;
	    
	    float dist = sqrt(dot(p-r, p-r));
	    if (dist > r) discard;
	    
	    return input.color;
	
	}
	return input.color;	
    }
    else if((0xFF & input.type) == 5 ) // Text
    {
	int slot = (0xFF00 & input.type) >> 8;
	float df;
	//df = textures[slot].Sample(samp, input.uv.xy).r;
	if (slot == 0) df = textures[0].Sample(samp, input.uv.xy).r;
	if (slot == 1) df = textures[1].Sample(samp, input.uv.xy).r;
	if (slot == 2) df = textures[2].Sample(samp, input.uv.xy).r;
	return df * input.color;
    }


    return float4(1.0f, 0.0f, 1.0f, 1.0f);


}
