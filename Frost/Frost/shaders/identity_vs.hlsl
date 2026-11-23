// Most basic shader, essentially just renders mesh blank

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorldViewProj;
    float4 time;
    float4 pulseColor;
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
    float4 time : TEXCOORD0;
    float4 pulseColor : TEXCOORD1;
};

OutputType VS(InputType input)
{
    OutputType output;
    
    // Transform to homogenous clip space
    input.pos.y += 0.5 * sin(input.pos.x + time.x * 3.f);
    output.pos = mul(float4(input.pos, 1.0f), gWorldViewProj);

    // Pass vertex colour to pixel shader
    output.color = input.color;
    output.time = time;
    output.pulseColor = pulseColor;
    
    return output;
}