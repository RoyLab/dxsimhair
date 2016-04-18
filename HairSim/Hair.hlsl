cbuffer cbMatrix
{
    matrix  g_mViewProjection;
    matrix  g_mWorld;
}

cbuffer cbLighting
{
    float3  direction;
}


struct VS_INPUT
{
    int    Sequence     : SEQ;
    float4 Position     : POSITION;
    float3 Color        : COLOR;
    float4 Direction    : DIRECTION;
};

struct VS_OUTPUT
{
    float4 Position     : SV_POSITION; // vertex position 
    float4 Color        : COLOR0;      // vertex diffuse color (note that COLOR0 is clamped from 0..1)
    float4 Direction    : DIRECTION0;
    float  Sequence     : SEQ0;
};

//--------------------------------------------------------------------------------------
// This shader computes standard transform and lighting
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT Output;

    // Transform the position from object space to homogeneous projection space
    Output.Position = mul(input.Position, g_mViewProjection);

    // Calc color    
    Output.Color = float4(input.Color, 0.0);

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
}
