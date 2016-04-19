cbuffer cbMatrix : register(b0)
{
    matrix  g_mViewProjection;
    matrix  g_mWorld;
    float   g_time;
}

struct VS_INPUT
{
    int    Sequence     : SEQ;
    float3 Position     : POSITION;
    float3 Color        : COLOR;
    float3 Direction    : DIRECTION;
};

struct VS_OUTPUT
{
    float4 Position     : SV_POSITION; // vertex position 
    float4 Color        : COLOR0;      // vertex diffuse color (note that COLOR0 is clamped from 0..1)
    float3 Direction    : DIRECTION0;
    float  Sequence     : SEQ0;
};

//--------------------------------------------------------------------------------------
// This shader computes standard transform and lighting
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT Output;

    // Transform the position from object space to homogeneous projection space
    Output.Position = mul(float4(input.Position, 1.0), g_mViewProjection);

    // Calc color    
    Output.Color = float4(input.Color, 1.0);
    Output.Sequence = float(input.Sequence);
    Output.Direction = input.Direction;

    return Output;
}

//--------------------------------------------------------------------------------------
// This shader outputs the pixel's color by modulating the texture's
// color with diffuse material color
//--------------------------------------------------------------------------------------
float4 PS(VS_OUTPUT In) : SV_TARGET
{
    // Lookup mesh texture and modulate it with diffuse
    return In.Color;
    //return float4(abs(In.Direction), 1.0) * (6 + abs(In.Sequence - 6)) / 25.0;
}
