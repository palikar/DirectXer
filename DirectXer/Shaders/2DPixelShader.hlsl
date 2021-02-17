struct PSIn
{
    float4 pos : SV_Position;
};

float4 main(PSIn input) : SV_Target
{

    return float4(1.0f, 0.0f, 0.0f, 1.0f);
}