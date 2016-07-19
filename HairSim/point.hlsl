////////////////////////////////////////////////////////////////////////////////
// Filename: shadow.vs
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////
cbuffer cbMatrix : register(b0)
{
    matrix  g_mWorldViewProjection;
    float3  g_viewPoint;
	float	g_size;
}


//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    int    Sequence		: SEQ;
    float3 Position     : POSITION;
    float3 Color        : COLOR;
    float3 Direction    : DIR;
    float3 Reference    : REF;
};

struct GeometryInputType
{
    float3 position     : POSITION;
    float4 color        : COLOR;
};


struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 color: COLOR0;
};

struct VS_OUTPUT
{
    float4 position     : SV_POSITION; // vertex position 
};

GeometryInputType VS(VertexInputType input)
{
    GeometryInputType output;

    output.position = input.Position;
    output.color = float4(input.Color, 1.0);

    return output;
}
////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 PS(PixelInputType input) : SV_TARGET
{
    return input.color;
}

PixelInputType VS2(VertexInputType input)
{
	PixelInputType output;
	float4 pos = float4(input.Position, 1.0);

	output.position = mul(pos, g_mWorldViewProjection);
	output.color = float4(input.Color, 1.0);

	return output;
}

[maxvertexcount(6)]
void GS(point GeometryInputType points[1], inout TriangleStream<PixelInputType> output)
{
    float3 p0 = points[0].position;
    float3 center = p0;
    float3 viewDir = normalize(g_viewPoint - center);
    float3 dir = normalize(viewDir+float3(0.1, 0.23,0.43));

    float3 normal = normalize(cross(dir, viewDir));
    float width = g_size;

    PixelInputType v[4];

    float3 dir_offset = dir * width;
    float3 normal_scaled = (-normal * width)/2;

    float3 p0_ex = p0 - dir_offset;
    float3 p1_ex = p0 + dir_offset;

	float4 position0;
    position0 = float4(p0_ex - normal_scaled, 1);
    v[0].position = mul(position0, g_mWorldViewProjection);
    v[0].color = points[0].color;

    position0 = float4(p0_ex + normal_scaled, 1);
    v[1].position = mul(position0, g_mWorldViewProjection);
    v[1].color = points[0].color;

    position0 = float4(p1_ex + normal_scaled, 1);
    v[2].position = mul(position0, g_mWorldViewProjection);
    v[2].color = points[0].color;

    position0 = float4(p1_ex - normal_scaled, 1);
    v[3].position = mul(position0, g_mWorldViewProjection);
    v[3].color = points[0].color;

    output.Append(v[2]);
    output.Append(v[1]);
    output.Append(v[0]);

    output.RestartStrip();

    output.Append(v[3]);
    output.Append(v[2]);
    output.Append(v[0]);

    output.RestartStrip();
}
