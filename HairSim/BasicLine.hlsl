cbuffer cbMatrix : register(b0)
{
    matrix  g_mViewProjection;
    matrix  g_mWorld;
}

struct VSInput
{
    float3 Position     : POSITION;
    float3 Color        : COLOR;
};

struct  VSOutput
{
    float4 Position     : SV_POSITION; 
    float3 Color        : COLOR0;
};

VSOutput VS(VertexInputType input)
{
    VSOutput Output;
    Output.Position = mul(float4(input.Position, 1.0), g_mViewProjection);
    Output.Color = float4(input.Color, 1.0);
    return Output;
}

float4 PS(VSOutput input) : SV_TARGET
{
    return input.Color;
}
