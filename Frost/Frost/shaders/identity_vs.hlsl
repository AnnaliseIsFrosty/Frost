// Most basic shader, essentially just renders mesh blank

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorldViewProj;
};

struct InputType
{
    float3 pos : POSITION;
    float4 color : COLOR;
};

struct OutputType
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

OutputType vs(InputType input)
{
    OutputType output;
    
    // Transform to homogenous clip space
    output.pos = mul(float4(input.pos, 1.0f), gWorldViewProj);

    // Pass vertex colour to pixel shader
    output.color = input.color;
    
    return output;
}