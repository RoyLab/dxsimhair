////////////////////////////////////////////////////////////////////////////////
// Filename: shadow.vs
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////
cbuffer cbMatrix : register(b0)
{
    matrix  g_mViewProjection;
    matrix  g_mWorld;
    float3  g_viewPoint;
    float   g_time;
    float2 g_renderTargetSize;
}


//////////////////////
// CONSTANT BUFFERS //
//////////////////////
cbuffer cbMatrix2 : register(b1)
{
    matrix g_lightProjView;
    int mode;
}

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    int    Sequence : SEQ;
    float3 Position     : POSITION;
    float3 Color        : COLOR;
    float3 Direction    : DIR;
    float3 Reference    : REF;
};

struct GeometryInputType
{
    int    sequence : SEQ;
    float3 position     : POSITION;
    float4 color        : COLOR;
    float3 direction    : DIR;
    float4 lightViewPosition : TEXCOORD1;
};


struct PixelInputType
{
    float4 position : SV_POSITION;
    float  Sequence : SEQ0;
    float3 direction : DIRECTION0;
    float4 color: COLOR0;
    float4 lightViewPosition : TEXCOORD1;
    float4 position0 : POSITION_0; // not reference, but orginal
};

struct VS_OUTPUT
{
    float4 position     : SV_POSITION; // vertex position 
};


float3 GetColour(float v)
{
    float3 c = { 1.0, 1.0, 1.0 }; // white
        float dv;

    if (v < 0.0)
        v = 0.0;
    if (v > 1.0)
        v = 1.0;
    dv = 1.0;

    if (v < (0.25 * dv)) {
        c.r = 0;
        c.g = 4 * (v) / dv;
    }
    else if (v < (0.5 * dv)) {
        c.r = 0;
        c.b = 1 + 4 * (0.25 * dv - v) / dv;
    }
    else if (v < (0.75 * dv)) {
        c.r = 4 * (v - 0.5 * dv) / dv;
        c.b = 0;
    }
    else {
        c.g = 1 + 4 * (0.75 * dv - v) / dv;
        c.b = 0;
    }

    return(c);
}


static const float piover180 = 0.0174532925f;
static const float pi = 3.141592653;
static const float e = 2.7182818282135;

static const float absorptionr = 0.54;
static const float absorptiong = 0.64;
static const float absorptionb = 0.8;
static const float eccentricity = 1.0;

static const float longshift_R = 3.0*piover180;
static const float longshift_TT = 1.5*piover180;
static const float longshift_TRT = 6 * piover180;

static const float longwidth_R = 2.0*piover180;
static const float longwidth_TT = 1.0*piover180;
static const float longwidth_TRT = 25.0*piover180;

static const float glintscale = 0.4;
static const float azimuthalwidth = 1.5*piover180;
static const float faderange = 0.3;
static const float causticintensity = 0.5;
static const float mu = 1.55;
static const float sqrt2pi = sqrt(2 * pi);

float rand_1_05(in float2 uv)
{
    float2 noise = (frac(sin(dot(uv, float2(12.9898, 78.233)*2.0)) * 43758.5453));
        return abs(noise.x + noise.y) * 0.5;
}

GeometryInputType VS(VertexInputType input)
{
    GeometryInputType output;
    float4 worldPosition;

    // Change the position vector to be 4 units for proper matrix calculations.
    float4 pos = float4(input.Position, 1.0);

        // Calculate the position of the vertex against the world, view, and projection matrices.
        output.position = pos;

    // Calculate the position of the vertice as viewed by the light source.
    output.lightViewPosition = mul(pos, g_lightProjView);

    // Calculate the normal vector against the world matrix only.
    output.direction = input.Direction;

    output.color = float4(input.Color, 1.0);
    if (mode == 4)
        output.color = float4(abs(input.Direction), 1.0);

    if (mode == 3)
    {
        float3 diff = input.Position - input.Color;
            float error = sqrt(dot(diff, diff)) * 3;
        output.color = saturate(float4(GetColour(error), 1.0));
    }

    if (mode == 5)
    {
        float3 diff = abs(input.Position - input.Reference);
            float error = sqrt(dot(diff, diff)) * 3;
        error = saturate(error);
        output.color *= (1 - error);
    }
    output.sequence = float(input.Sequence);

    output.color.w = 1.0;
    return output;
}

////////////////////////////////////////////////////////////////////////////////
// Filename: shadow.ps
////////////////////////////////////////////////////////////////////////////////


//////////////
// TEXTURES //
//////////////
Texture2D shaderTexture : register(t0);


///////////////////
// SAMPLE STATES //
///////////////////
SamplerState SampleTypeClamp : register(s0);


float normalFromNegPiToPi(float value)
{
    if ((value <= pi) == (value >= -pi)) return value;
    float twopi = 2 * pi;
    value += pi;
    float count = floor(value / twopi);
    return value - count * twopi - pi;
}

float normalFromZeroTo2Pi(float v)
{
    float twopi = 2 * pi;
    if ((v <= twopi) == (v >= 0)) return v;

    float count = floor(v / twopi);
    return v - count * twopi;
}

float testsolve(float x)
{
    if (pi / 2 >= x && x >= -pi / 2)return true;
    else return false;
}

float Gaussian(float sdeviation, float x)
{
    return (1.0 / (sqrt2pi*sdeviation)) * pow(e, -x*x / (2 * sdeviation*sdeviation));
}

float smoothstep(float a, float b, float x)
{
    if (x < a)return 1;
    if (x > b)return 0;
    else return (x - b) / (a - b);
}

float cubeRoot(float d) {
    if (d < 0.0) {
        return -pow(-d, 1.0 / 3.0);
    }
    else {
        return pow(d, 1.0 / 3.0);
    }
}

float solveTT(float c0, float phi)
{
    float a = -8.0*c0 / pow(pi, 3);
    float p = (6.0*c0 / pi - 2) / a;
    float q = (pi - phi) / a;

    float q_div_2 = -q / 2;
    float delta = sqrt(q_div_2 * q_div_2 + p*p*p / 27.0);
    return cubeRoot(q_div_2 + delta) + cubeRoot(q_div_2 - delta);
}
//

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 PS(PixelInputType input) : SV_TARGET
{
    return input.color;
}

[maxvertexcount(6)]
void GS(point GeometryInputType points[1], inout TriangleStream<PixelInputType> output)
{
    float3 p0 = points[0].position;
    float3 center = p0;
    float3 viewDir = normalize(g_viewPoint - center);
    float3 dir = normalize(viewDir+float3(0.1, 0.23,0.43));

    float3 normal = normalize(cross(dir, viewDir));
    float width = 0.005f;

    PixelInputType v[4];

    float3 dir_offset = dir * width;
    float3 normal_scaled = -normal * width;

    float3 p0_ex = p0 - dir_offset;
    float3 p1_ex = p0 + dir_offset;

    v[0].position0 = float4(p0_ex - normal_scaled, 1);
    v[0].position = mul(v[0].position0, g_mViewProjection);
    v[0].color = points[0].color;
    v[0].Sequence = points[0].sequence;
    v[0].direction = points[0].direction;
    v[0].lightViewPosition = points[0].lightViewPosition;

    v[1].position0 = float4(p0_ex + normal_scaled, 1);
    v[1].position = mul(v[1].position0, g_mViewProjection);
    v[1].color = points[0].color;
    v[1].Sequence = points[0].sequence;
    v[1].direction = points[0].direction;
    v[1].lightViewPosition = points[0].lightViewPosition;

    v[2].position0 = float4(p1_ex + normal_scaled, 1);
    v[2].position = mul(v[2].position0, g_mViewProjection);
    v[2].color = points[0].color;
    v[2].Sequence = points[0].sequence;
    v[2].direction = points[0].direction;
    v[2].lightViewPosition = points[0].lightViewPosition;

    v[3].position0 = float4(p1_ex - normal_scaled, 1);
    v[3].position = mul(v[3].position0, g_mViewProjection);
    v[3].color = points[0].color;
    v[3].Sequence = points[0].sequence;
    v[3].direction = points[0].direction;
    v[3].lightViewPosition = points[0].lightViewPosition;

    output.Append(v[2]);
    output.Append(v[1]);
    output.Append(v[0]);

    output.RestartStrip();

    output.Append(v[3]);
    output.Append(v[2]);
    output.Append(v[0]);

    output.RestartStrip();
}
