
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
    float3 CameraPos;
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
    float4 AmbientLightColor;

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


float3 ApplyAmbientLight(float3 ambientReflectance)
{
    return AmbientLightColor.a * AmbientLightColor.rgb * ambientReflectance;
}

float3 ApplyDirLight(float3 normal, float3 diffuseFactor)
{
    const float3 light_to_surface = normalize(DirLightDir).xyz;
    const float diffuse_coefficient = max(0.0, dot(normal, light_to_surface));
    float3 diffuse = diffuse_coefficient * diffuseFactor.rgb * DirLightColor.rgb * DirLightColor.a;
    return diffuse;
}

float3 ApplyPointLight(PointLight light, float3 normal, float3 surface_pos, float3 surface_to_camera,
    float3 diffuseFactor, float3 specularFactor, float specularExponent)
{
    float3 light_to_surface  = normalize(light.Position.xyz - surface_pos).xyz;
    float dist_to_light = length(light.Position.xyz - surface_pos);

    float attenuation = 1.0f / (light.Params.r + light.Params.g * dist_to_light +
                                light.Params.b * (dist_to_light * dist_to_light));


    float diffuse_coefficient = max(0.0, dot(normal, light_to_surface));
    float3 diffuse = diffuse_coefficient * diffuseFactor.rgb * light.Color.rgb;

    float specular_coefficient = 0.0;
    if(diffuse_coefficient > 0.0)
        specular_coefficient = pow(max(0.0, dot(surface_to_camera, reflect(-light_to_surface, normal))), specularExponent);
    float3 specular = specularFactor.rgb * light.Color.rgb * 1.0f;

    return (diffuse + specular) * attenuation;
}

VSOut main( VSIn input)
{
    VSOut output = (VSOut)0;

    output.pos = mul(float4(input.pos.x, input.pos.y, input.pos.z, 1.0), mul(model, mul(view, projection)));

    float3 worldPos = (float3)mul(float4(input.pos.x, input.pos.y, input.pos.z, 1.0), model);
    float3 toCamera = -normalize(worldPos - CameraPos);

    float3 finalColor = float3(0.0f, 0.0f, 0.0f);
    float3 diffuseFactor = KdMap.SampleLevel(pointSamp, input.uv, 0).rgb;

    finalColor += ApplyAmbientLight(Ka);
    finalColor += ApplyDirLight(input.normal, diffuseFactor);

    for (int i = 0; i < 5; ++i)
    {
	if (PointLights[i].active)
	{
	    finalColor += ApplyPointLight(PointLights[i], input.normal, worldPos, toCamera,
					  diffuseFactor, Ks, Ns);
	}
    }

    output.color = finalColor;
    return output;
}
