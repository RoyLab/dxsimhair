cbuffer cbMatrix : register(b0)
{
    matrix  g_mViewProjection;
    matrix  g_mWorld;
}

struct VSInput
{
    float3 Position     : POSITION1;
    float3 Color        : COLOR;
};

struct  VSOutput
{
    float4 Position     : SV_POSITION; 
    float3 Color        : COLOR0;
};

VSOutput VS(float3 Position: POSITION)
{
    VSOutput Output;
    Output.Position = mul(float4(Position, 1.0), g_mViewProjection);
    return Output;
}

float4 PS(VSOutput input) : SV_TARGET
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
