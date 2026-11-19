// Most basic shader, essentially just renders mesh blank

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorldViewProj;
};

struct InputType
{
    float3 pos : SV_POSITION;
    float4 color : COLOR;
};

float4 ps(InputType input)
{
    return input.color;
}