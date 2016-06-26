//#include "Include/ShaderStates.fx"

Texture2D LineTexture;

//-----------------------------------------------------------------------------
// Constant Buffers
//-----------------------------------------------------------------------------
cbuffer cbChangeRare
{
    float2 RenderTargetSize;
}

cbuffer cbChangePerFrame
{
}

cbuffer cbChangePerObject
{
    matrix WorldViewProjection;
}

//-----------------------------------------------------------------------------
// Shader Input / Output Structures
//-----------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Position : POSITION0;
};

struct GEO_IN
{
    float4 Position : POSITION0;
};

struct GEO_OUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

struct PS_OUTPUT
{
    float4 Color : SV_Target0;
};

GEO_IN VS_LineV2(in VS_INPUT input)
{
    GEO_IN output;
    output.Position = mul(input.Position, WorldViewProjection);
    //output.Position = input.Position;
    return output;
}

[maxvertexcount(4)]
void GS(line GEO_IN points[2], inout TriangleStream<GEO_OUT> output)
{
    float4 p0 = points[0].Position;
        float4 p1 = points[1].Position;

        float w0 = p0.w;
    float w1 = p1.w;

    p0.xyz /= p0.w;
    p1.xyz /= p1.w;

    float3 line01 = p1 - p0;
        float3 dir = normalize(line01);

        // scale to correct window aspect ratio
        float3 ratio = float3(RenderTargetSize.y, RenderTargetSize.x, 0);
        ratio = normalize(ratio);

    float3 unit_z = normalize(float3(0, 0, -1));

        float3 normal = normalize(cross(unit_z, dir) * ratio);

        float width = 0.01;

    GEO_OUT v[4];

    float3 dir_offset = dir * ratio * width;
        float3 normal_scaled = normal * ratio * width;

        float3 p0_ex = p0 - dir_offset;
        float3 p1_ex = p1 + dir_offset;

        v[0].Position = float4(p0_ex - normal_scaled, 1) * w0;
    v[0].TexCoord = float2(0, 0);

    v[1].Position = float4(p0_ex + normal_scaled, 1) * w0;
    v[1].TexCoord = float2(0, 0);

    v[2].Position = float4(p1_ex + normal_scaled, 1) * w1;
    v[2].TexCoord = float2(0, 0);

    v[3].Position = float4(p1_ex - normal_scaled, 1) * w1;
    v[3].TexCoord = float2(0, 0);

    output.Append(v[2]);
    output.Append(v[1]);
    output.Append(v[0]);

    output.RestartStrip();

    output.Append(v[3]);
    output.Append(v[2]);
    output.Append(v[0]);

    output.RestartStrip();
}

PS_OUTPUT PS_LineV2(GEO_OUT input)
{
    PS_OUTPUT output;

    //output.Color = LineTexture.Sample(LinearSampler, input.TexCoord);
    //output.Color = float4(input.TexCoord.xy, 0, 1);
    output.Color = float4(1, 0.5, 0, 0.5);

    return output;
}

technique10 LineV2Technique
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_4_0, VS_LineV2()));
        SetGeometryShader(CompileShader(gs_4_0, GS()));
        SetPixelShader(CompileShader(ps_4_0, PS_LineV2()));
    }
}