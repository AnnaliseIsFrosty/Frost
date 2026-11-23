// Most basic shader, essentially just renders mesh blank

struct InputType
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
    float4 time : TEXCOORD0;
    float4 pulseColor : TEXCOORD1;
};

float4 PS(InputType input) : SV_Target
{
    float4 outputCol = input.color;
    outputCol = lerp(input.color, input.pulseColor, (sin(input.time.x * 3) + 1) / 2);
    //outputCol += input.pulseColor * ((sin(input.time.x * 3) + 1) / 2);
    //outputCol /= 2;
    return outputCol;
}