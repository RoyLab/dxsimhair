////////////////////////////////////////////////////////////////////////////////
// Filename: depth.vs
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////
cbuffer cbMatrix : register(b0)
{
    matrix  g_mViewProjection;
    matrix  g_mWorld;
    float   g_time;
}

cbuffer cbMatrix2 : register(b1)
{
    matrix g_lightProjView;
}

struct VS_INPUT
{
    int    Sequence : SEQ;
    float3 Position     : POSITION;
    float4 Color        : COLOR;
    float4 Direction    : DIRECTION;
};

struct VS_OUTPUT
{
    float4 Position     : SV_POSITION; // vertex position 
};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;

    float4 pos = float4(input.Position, 1.0f);
    output.Position = mul(g_lightProjView, pos);
	
	return output;
}

float PS(VS_OUTPUT In) : SV_TARGET
{
    return In.Position.z;
}
