

struct PSIn
{
    float4 pos : SV_Position;
    float3 world_pos : PositionOut;
    float2 uv : Texcoord;
    float3 color : COLOR0;
};

cbuffer CbBuf : register(b0)
{
    float3 CameraPos;
    uint shaderType;
};

cbuffer TexturedMaterialBuf : register(b1)
{
    float4 Color;
    float ColorIntensity;
    float AoIntensity;
    float Reflectivity;
    float RefractionRation;  
};


SamplerState samp;

TextureCube env : register(t0);
Texture2D tex_1 : register(t1);
Texture2D tex_2 : register(t2);

float4 main(PSIn input) : SV_Target
{
    if (shaderType == 0) // Tex
    {

	float3 finalColor = tex_1.Sample(samp, input.uv).rgb;
	finalColor = lerp(finalColor, Color, ColorIntensity);

	float ambientStrength = (tex_2.Sample(samp, input.uv).r - 1.0) * AoIntensity + 1.0;
	finalColor *= ambientStrength;

	float3 I = normalize(input.world_pos - CameraPos);
	float3 envColor = float3(0,0,0);
	/* if(Reflectivity > 0.0f) */
	/* { */
	/*     float3 R = reflect(I, normalize(normal)); */
	/*     envColor = env.Sample(samp, R).rgb; */
	/* } */
	
	/* if(RefractionRation > 0.0f) */
	/* { */
	/*     float3 R = refract(I, normalize(normal), RefractionRation); */
	/*     envColor = env.Samle(samp, R).rgb; */
	/* } */
	/* finalColor = lerp(finalColor, envColor, Reflectivity) ; */
	
	return float4(finalColor, 1.0f);
    }
    else if(shaderType == 1) // Color
    {
	return float4(input.color, 1.0f);
    }
    else if(shaderType == 2) // Skybox
    {
        return env.Sample(samp, input.world_pos);
    }

    return float4(1.0f, 0.0f, 0.5f, 1.0f);
}
