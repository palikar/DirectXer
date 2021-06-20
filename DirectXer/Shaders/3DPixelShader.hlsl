struct PSIn
{
    float4 pos : SV_Position;
    float4 color : Color;
};

float4 main(PSIn input) : SV_Target
{
    return input.color;
}
