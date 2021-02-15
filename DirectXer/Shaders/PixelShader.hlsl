struct PSIn
{
    float4 pos : SV_Position;
    float3 world_pos : PositionOut;
    float2 uv : Texcoord;
    float3 color : COLOR0;
    float3 normal: Normal;
};


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

    PointLight PointLights[5];
    SpotLight SpotLights[5];
    
};

cbuffer PhongMaterialBuf : register(b3)
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float4 Emissive;
    float SpecularShininess;
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

float3 apply_point_light(PointLight light, float3 normal, float3 surface_pos, float3 surface_to_camera)
{
    float3 light_to_surface  = normalize(light.Position - surface_pos);
    float dist_to_light = length(light.Position - surface_pos);

    float attenuation = 1.0f / (light.Params.r + light.Params.g * dist_to_light +
                                light.Params.b * (dist_to_light * dist_to_light));


    float diffuse_coefficient = max(0.0, dot(normal, light_to_surface));
    float3 diffuse = diffuse_coefficient * Diffuse.rgb * light.Color.rgb;

    float specular_coefficient = 0.0;
    if(diffuse_coefficient > 0.0)
        specular_coefficient = pow(max(0.0, dot(surface_to_camera, reflect(-light_to_surface, normal))), SpecularShininess);
    float3 specular = Specular.rgb * light.Color.rgb * 1.0f;

    return (diffuse + specular) * attenuation;
}

float3 apply_spot_light(SpotLight light, float3 normal, float3 surface_pos, float3 surface_to_camera)
{
    float3 light_to_surface  = normalize(light.Position - surface_pos);
    float theta = acos(dot(light_to_surface, normalize(-light.Dir)));

    if(theta < light.Params.r) {
        float epsilon  = light.Params.g - light.Params.r;
        float intensity = clamp((theta - light.Params.r) / epsilon, 0.0, 1.0);
        /* float distance_to_light = length(light.position - surface_pos); */

        float diffuse_coefficient = max(0.0, dot(normal, light_to_surface));
        float3 diffuse = Diffuse.rgb * light.Color.rgb * diffuse_coefficient;

        float specular_coefficient = 0.0;
        if(diffuse_coefficient > 0.0) {
            specular_coefficient = pow(max(0.0, dot(surface_to_camera, reflect(-light_to_surface, normal))), SpecularShininess);
        }
        float3 specular = specular_coefficient * Specular.rgb * light.Color.rgb;

        return (diffuse + specular) * intensity;
    }
    return float3(0.0, 0.0, 0.0);

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


	float3 to_camera = -normalize(input.world_pos - CameraPos);
	
	float3 outgoingLight = float3(0.0f, 0.0f, 0.0f);

	outgoingLight += apply_ambient_light(input);
        outgoingLight += apply_dir_light(input);

	for (int i = 0; i < 5; ++i)
	{
	    if (PointLights[i].active)
	    {
		outgoingLight += apply_point_light(PointLights[i], input.normal, input.world_pos, to_camera);
	    }

	    if (SpotLights[i].active)
	    {
		outgoingLight += apply_spot_light(SpotLights[i], input.normal, input.world_pos, to_camera);
	    }
	}


	return float4(outgoingLight, 1.0f);
    }

    return float4(1.0f, 0.0f, 0.5f, 1.0f);
}
