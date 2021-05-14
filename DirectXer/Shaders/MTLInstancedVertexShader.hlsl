#include "MTLCommon.ihlsl"

struct VSIn
{
    float3 pos : Position;
    float2 uv : Texcoord;
    float3 normal : Normal;

    matrix modelMatrix: Model;
    matrix invmodelMatrix: InvModel;
};

VSOut main( VSIn input)
{
    VSOut output = (VSOut)0;

    output.pos = mul(float4(input.pos.x, input.pos.y, input.pos.z, 1.0), mul(input.modelMatrix, mul(view, projection)));
    float3 normal = mul(transpose(input.invmodelMatrix), input.normal).xyz;

    float3 worldPos = (float3)mul(float4(input.pos.x, input.pos.y, input.pos.z, 1.0), model);
    float3 toCamera = -normalize(worldPos - CameraPos);

    output.color = Shade(worldPos, toCamera, input.uv, normal);
    return output;
}
