// Most basic shader, essentially just renders mesh blank

struct InputType
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

float4 PS(InputType input) : SV_Target
{
    return input.color;
}