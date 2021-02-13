struct PSIn
{
    float4 pos : SV_Position;
    float3 world_pos : PositionOut;
    float2 uv : Texcoord;
    float3 color : COLOR0;
    float3 normal: Normal;
};

cbuffer PSPrimBuf : register(b0)
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

cbuffer LightningBuf : register(b2)
{
    float4 AmbLightColor;

    float4 DirLightColor;
    float4 DirLightDir;
};

cbuffer PhongMaterialBuf : register(b3)
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float4 Emissive;
};


SamplerState samp;

TextureCube env : register(t0);
Texture2D tex_1 : register(t1);
Texture2D tex_2 : register(t2);


float3 apply_ambient_light(in PSIn input)
{
    return AmbLightColor.a * AmbLightColor.rgb * Ambient.rgb;
}

float3 apply_dir_light(in PSIn input)
{
    const float3 light_to_surface  = normalize(DirLightDir);
    const float diffuse_coefficient = max(0.0, dot(input.normal, light_to_surface));
    float3 diffuse = diffuse_coefficient * Diffuse.rgb * DirLightColor.rgb * DirLightColor.a;
    return diffuse;
}


float4 main(PSIn input) : SV_Target
{
    if (shaderType == 0) // Tex
    {

	float3 finalColor = tex_1.Sample(samp, input.uv).rgb;
	finalColor = lerp(finalColor, Color.rgb, ColorIntensity);

	float ambientStrength = (tex_2.Sample(samp, input.uv).r - 1.0) * AoIntensity + 1.0;
	finalColor *= ambientStrength;

	float3 I = normalize(CameraPos - input.world_pos );
	float3 envColor = float3(0,0,0);

	if(Reflectivity > 0.0f)
	{
	    float3 R = reflect(I, normalize(input.normal));
	    envColor = env.Sample(samp, R).rgb;
	}

	if(RefractionRation > 0.0f)
	{
	    float3 R = refract(I, normalize(input.normal), RefractionRation);
	    envColor = env.Sample(samp, R).rgb;
	}


	finalColor = lerp(finalColor, envColor, Reflectivity) ;

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
    else if(shaderType == 3) // Simple texture
    {
	return tex_1.Sample(samp, input.uv);
    }
    else if(shaderType == 4) // Phong
    {



	float3 outgoingLight = float3(0.0f, 0.0f, 0.0f);

	outgoingLight += apply_ambient_light(input);
        outgoingLight += apply_dir_light(input);

	return float4(outgoingLight, 1.0f);
    }

    return float4(1.0f, 0.0f, 0.5f, 1.0f);
}
