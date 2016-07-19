// constant buffers
cbuffer cbMatrix : register(b0)
{
    matrix  g_mProjViewWorld;
    matrix  g_mLightProjViewWorld;
    float3  g_viewPoint;
    int     mode; // 0 use color, 1 use difference
}


// typedefs
struct VertexInputType
{
    int    Sequence     : SEQ;
    float3 Position     : POSITION;
    float3 Color        : COLOR;
    float3 Direction    : DIR;
    float3 Reference    : REF;
};

struct GeometryInputType
{
    int    sequence     : SEQ;
    float3 position     : POSITION;
    float3 color        : COLOR;
    float3 direction    : DIR;
    float4 lightViewPosition : TEXCOORD1;
};


struct PixelInputType
{
    float4 position     : SV_POSITION;
    float  Sequence     : SEQ0;
    float3 direction    : DIRECTION0;
    float3 color        : COLOR0;
    float4 lightViewPosition : TEXCOORD1;
    float4 position0    : POSITION_0; // not reference, but orginal
};

struct VS_OUTPUT
{
    float4 position     : SV_POSITION; // vertex position 
};


VS_OUTPUT SM_VS(VertexInputType input)
{
    VS_OUTPUT output;
    output.position = mul(float4(input.Position, 1.0f), g_mLightProjViewWorld);

    return output;
}

float SM_PS(VS_OUTPUT In) : SV_TARGET
{
    return In.position.z;
}

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

    output.position = input.Position;
    output.lightViewPosition = mul(float4(input.Position, 1.0), g_mLightProjViewWorld);
    output.direction            = input.Direction;
    output.color                = abs(input.Color);

    if (mode == 1)
    {
        float3 diff = input.Position - input.Reference;
        float error = sqrt(dot(diff, diff)) * 3;
        output.color = saturate(GetColour(error));
    }

    output.sequence = float(input.Sequence);
    return output;
}


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

float F(float i1, float i2, float gi, float gt)
{
    float F_p, F_s;
    F_p = (i2*cos(gi) - cos(gt)) / (i2*cos(gi) + cos(gt));
    F_s = (cos(gi) - i1*cos(gt)) / (cos(gi) + i1*cos(gt));
    return (F_p*F_p + F_s*F_s) / 2;
}

float N_R(float phi, float mu_1, float mu_2)
{
    float gamma_i = -phi / 2;
    float h = sin(gamma_i);
    float gamma_t = asin(h / mu_1);
    return F(mu_1, mu_2, gamma_i, gamma_t) / abs(-2.0 / sqrt(1 - h*h));
}

float N_TT(float phi, float mu_1, float mu_2, float c)
{
    float normPhi = normalFromZeroTo2Pi(phi);
    float theta_i = solveTT(c, normPhi);
    float h = sin(theta_i);
    float theta_t = asin(h / mu_1);
    float Fa = F(mu_1, mu_2, theta_i, theta_t);
    float F1_a = 1 - Fa;
    //float atten = pow(e, -(2 * absorptionr / cos(theta_t))*(1 + cos(2 * gamma_t)));
    float atten = 1.0f;
    return F1_a*F1_a*atten / abs((2.0 / mu_1) / sqrt(1 - h*h / (mu_1*mu_1)) - 2.0 / sqrt(1 - h*h));
}

float M_R(float theta_h)
{
    return Gaussian(longwidth_R, (theta_h - longshift_R));
}

float M_TT(float theta_h)
{
    return Gaussian(longwidth_TT, theta_h - longshift_TT);
}

float3 scattering(float Phi_i, float Phi_r, float Theta_i, float Theta_r, float3 rgb)
{
    float phi_i = Phi_i;
    float phi_r = Phi_r;
    float phi = phi_r - phi_i;
    float phi_h = (phi_r + phi_i) / 2;

    float theta_i = Theta_i;
    float theta_r = Theta_r;
    float theta_h = (theta_i + theta_r) / 2;
    float theta_d = (theta_r - theta_i) / 2;

    float mu_1 = sqrt(mu*mu - sin(theta_i)*sin(theta_i)) / cos(theta_i);
    float mu_2 = mu*mu*cos(theta_i) / sqrt(mu*mu - sin(theta_i)*sin(theta_i));

    float c = asin(1.0f / mu_1); // < pi/2

    float m_r = M_R(theta_h);
    float n_r = N_R(phi, mu_1, mu_2);

    float m_tt = M_TT(theta_h);
    float n_tt = N_TT(phi, mu_1, mu_2, c);

    float cos2x = cos(theta_d)*cos(theta_d);

    return rgb * (n_r * m_r / cos2x +
        m_tt*n_tt / cos2x / 10);
    //    M_TRT()*N_TRT() / (cos(theta_d)*cos(theta_d)) * 250;
}


Texture2D shaderTexture : register(t0);
SamplerState SampleTypeClamp : register(s0);

float4 PS(PixelInputType input) : SV_TARGET
{
    //return float4(input.color, 1.0f);
    float bias;
    float3 color;
    float2 projectTexCoord;
    float depthValue;
    float lightDepthValue;

    // Calculate phi, theta
    float3 vecU = input.direction;
    float3 vecV = cross(vecU, float3(1, 0, 0));
    if (dot(vecV, vecV) < 1e-10)
        vecV = cross(vecU, float3(0, 1, 0));

    vecV = normalize(vecV);
    float3 vecW = cross(vecU, vecV);
    float3 viewVec = normalize(input.position0.xyz - g_viewPoint);

    //light direction (1, 1, -1)
    float3 lightVec = { 1 / 1.732, 1 / 1.732, -1 / 1.732 };
    float theta_i = -acos(dot(-lightVec, vecU)) + pi / 2;
    float theta_r = -acos(dot(-viewVec, vecU)) + pi / 2;
    float phi_i = acos(dot(-lightVec, vecV));
    if (dot(-lightVec, vecW) < 0) phi_i = -phi_i;

    float phi_r = acos(dot(-viewVec, vecV));
    if (dot(-viewVec, vecW) < 0) phi_r = -phi_r;

    float3 diffuse = scattering(phi_i, phi_r, theta_i, theta_r, input.color.rgb);
	//diffuse = input.color.rgb;

    // Set the bias value for fixing the floating point precision issues.
    bias = 0.001f;

    // Set the default output color to the ambient light value for all pixels.
    color = 0.05F * input.color; // ambient

    // Calculate the projected texture coordinates.
    projectTexCoord.x = input.lightViewPosition.x / input.lightViewPosition.w / 2.0f + 0.5f;
    projectTexCoord.y = -input.lightViewPosition.y / input.lightViewPosition.w / 2.0f + 0.5f;

    // Determine if the projected coordinates are in the 0 to 1 range.  If so then this pixel is in the view of the light.
    if ((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y))
    {
        // Sample the shadow map depth value from the depth texture using the sampler at the projected texture coordinate location.
        int w = 1024, h = 704;
        float cw = w * projectTexCoord.x;
        float ch = h * projectTexCoord.y;

        int2 orgin = int2(int(floor(cw)), int(floor(ch)));
            float dvs0 = shaderTexture.Load(int3(orgin.x, orgin.y, 0)).r;
        float dvs1 = shaderTexture.Load(int3(orgin.x, orgin.y + 1, 0)).r;
        float dvs2 = shaderTexture.Load(int3(orgin.x + 1, orgin.y, 0)).r;
        float dvs3 = shaderTexture.Load(int3(orgin.x + 1, orgin.y + 1, 0)).r;
        float4 dvs = float4(dvs0, dvs1, dvs2, dvs3);

        //depthValue = shaderTexture.Sample(SampleTypeClamp, projectTexCoord).r;

        // Calculate the depth of the light.
        lightDepthValue = input.lightViewPosition.z / input.lightViewPosition.w;

        // Subtract the bias from the lightDepthValue.
        lightDepthValue = lightDepthValue - bias;

        bool result[4];
        for (int i = 0; i < 4; i++)
            result[i] = (lightDepthValue < dvs[i]) ? 1.0 : 0.0;

        float w1 = (orgin.x + 1 - cw) * result[0] + (cw - orgin.x) * result[2];
        float w2 = (orgin.x + 1 - cw) * result[1] + (cw - orgin.x) * result[3];
        float weight = (orgin.y + 1 - ch) * w1 + (ch - orgin.y) * w2;

        // 相当于light的方向是（1，1，-1）
        diffuse *= weight;
        color += diffuse;
        color += 0.8 * weight * input.color.rgb * (1 - abs(input.direction.x + input.direction.y - input.direction.z) / 1.732);
        // Saturate the final light color.
        color = saturate(color);
    }

    return float4(color, 1.0);
}

[maxvertexcount(6)]
void GS(line GeometryInputType points[2], inout TriangleStream<PixelInputType> output)
{
    float3 p0 = points[0].position;
    float3 p1 = points[1].position;

    float3 line01 = (p1 - p0);
    float3 dir = normalize(line01);

    float3 center = (p1 + p0) / 2.0f;
    float3 viewDir = normalize(g_viewPoint - center);

    float3 normal = normalize(cross(dir, viewDir));
    float width = 0.005f;

    PixelInputType v[4];

    float3 dir_offset = dir * width;
    float3 normal_scaled = -normal * width;

    float3 p0_ex = p0 - 0;
    float3 p1_ex = p1 + 0;

    v[0].position0 = float4(p0_ex - normal_scaled, 1);
    v[0].position = mul(v[0].position0, g_mProjViewWorld);
    v[0].color = points[0].color;
    v[0].Sequence = points[0].sequence;
    v[0].direction = points[0].direction;
    v[0].lightViewPosition = points[0].lightViewPosition;

    v[1].position0 = float4(p0_ex + normal_scaled, 1);
    v[1].position = mul(v[1].position0, g_mProjViewWorld);
    v[1].color = points[0].color;
    v[1].Sequence = points[0].sequence;
    v[1].direction = points[0].direction;
    v[1].lightViewPosition = points[0].lightViewPosition;

    v[2].position0 = float4(p1_ex + normal_scaled, 1);
    v[2].position = mul(v[2].position0, g_mProjViewWorld);
    v[2].color = points[1].color;
    v[2].Sequence = points[1].sequence;
    v[2].direction = points[1].direction;
    v[2].lightViewPosition = points[1].lightViewPosition;

    v[3].position0 = float4(p1_ex - normal_scaled, 1);
    v[3].position = mul(v[3].position0, g_mProjViewWorld);
    v[3].color = points[1].color;
    v[3].Sequence = points[1].sequence;
    v[3].direction = points[1].direction;
    v[3].lightViewPosition = points[1].lightViewPosition;

    output.Append(v[2]);
    output.Append(v[1]);
    output.Append(v[0]);

    output.RestartStrip();

    output.Append(v[3]);
    output.Append(v[2]);
    output.Append(v[0]);

    output.RestartStrip();
}
