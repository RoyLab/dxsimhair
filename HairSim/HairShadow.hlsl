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

struct PixelInputType
{
    float4 position : SV_POSITION;
    float  Sequence : SEQ0;
    float3 direction : DIRECTION0;
    float4 color: COLOR0;
    float4 lightViewPosition : TEXCOORD1;
    float4 position0 : POSITION_0;
    //float theta_i: THETA_I;
    //float theta_r: THETA_R;
    //float phi_i: PHI_I;
    //float phi_r: PHI_R;
};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////


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

PixelInputType VS(VertexInputType input)
{
    PixelInputType output;
    float4 worldPosition;

    // Change the position vector to be 4 units for proper matrix calculations.
    float4 pos = float4(input.Position, 1.0);

    // Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(pos, g_mViewProjection);
    output.position0 = pos;

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
        float error = sqrt(dot(diff, diff))*3;
        error = saturate(error);
        output.color *= (1 - error);
    }
    output.Sequence = float(input.Sequence);

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


float trinormal(float x)
{
    while (!(pi >= x && x >= -pi))
    {
        if (-pi > x)x = x + 2 * pi;
        else if (x > pi) x = x - 2 * pi;
    }
    return x;
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

float solveTT(float c, float phi)
{
    float p = 1;
    float a = -8.0*p*c / pow(pi, 3);
    float b = 0;
    float c2 = 6.0*p*c / pi - 2;
    float d = -p*pi - phi;
    float delta = pow(b*c2 / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a), 2) + pow(c2 / (3.0*a) - b*b / (9.0*a*a), 3);
    float root1;
    if ((b*c2 / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a) - sqrt(delta)) < 0)
        root1 = -b / (3.0*a) +
        pow(b*c2 / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a) + sqrt(delta), 1.0 / 3.0) -
        pow(abs(b*c2 / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a) - sqrt(delta)), 1.0 / 3);
    else
        root1 = -b / (3.0*a) + 
        pow(b*c2 / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a) + sqrt(delta), 1.0 / 3.0) +
        pow(b*c2 / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a) - sqrt(delta), 1.0 / 3);

    return root1;
}
//
//float solve(int p)
//{
//    if (p == 0)
//    {
//        return -phi / 2;
//    }
//    else if (p == 1)
//    {
//        float a = -8.0*p*c / pow(pi, 3);
//        float b = 0;
//        float c = 6.0*p*c / pi - 2;
//        float d = -p*pi - phi;
//        float delta = pow(b*c / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a), 2) + pow(c / (3.0*a) - b*b / (9.0*a*a), 3);
//        float root1 = 0;
//        if ((b*c / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a) - sqrt(delta)) < 0)root1 = -b / (3.0*a) + pow(b*c / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a) + sqrt(delta), 1.0 / 3.0) - pow(abs(b*c / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a) - sqrt(delta)), 1.0 / 3);
//        else root1 = -b / (3.0*a) + pow(b*c / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a) + sqrt(delta), 1.0 / 3.0) + pow(b*c / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a) - sqrt(delta), 1.0 / 3);
//        return root1;
//    }
//    else
//    {
//        float a = -8.0*p*c / pow(pi, 3);
//        float b = 0;
//        float c = 6.0*p*c / pi - 2;
//        float d = -p*pi - phi;
//        float delta = pow(b*c / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a), 2) + pow(c / (3.0*a) - b*b / (9.0*a*a), 3);
//        if (delta > 0)return 1;
//        else return 3;
//    }
//}
//
//float solve11(float p)
//{
//    float a = -8.0*p*c / pow(pi, 3);
//    float b = 0;
//    float c = 6.0*p*c / pi - 2;
//    float d = p*pi - phi;
//    float delta = pow(b*c / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a), 2) + pow(c / (3.0*a) - b*b / (9.0*a*a), 3);
//    float root1 = 0;
//    if ((b*c / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a) - sqrt(delta)) < 0)root1 = -b / (3.0*a) + pow(b*c / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a) + sqrt(delta), 1.0 / 3.0) - pow(abs(b*c / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a) - sqrt(delta)), 1.0 / 3);
//    else root1 = -b / (3.0*a) + pow(b*c / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a) + sqrt(delta), 1.0 / 3.0) + pow(b*c / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a) - sqrt(delta), 1.0 / 3);
//    return root1;
//}

//float solve1()
//{
//    float p = 2.0;
//    float a = -8.0*p*c / pow(pi, 3);
//    float b = 0;
//    float c = 6.0*p*c / pi - 2;
//    float d = -p*pi - phi;
//    float alpha = b*c / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a);
//    float beta = c / (3.0*a) - b*b / (9.0*a*a);
//    return -b / (3 * a) + 2 * sqrt(-beta)*cos(acos(alpha / pow(-beta, 3.0 / 2.0)) / 3);
//}

//float solve2()
//{
//    float p = 2.0;
//    float a = -8.0*p*c / pow(pi, 3);
//    float b = 0;
//    float c = 6.0*p*c / pi - 2;
//    float d = -p*pi - phi;
//    float alpha = b*c / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a);
//    float beta = c / (3.0*a) - b*b / (9.0*a*a);
//    return -b / (3 * a) + 2 * sqrt(-beta)*cos((acos(alpha / pow(-beta, 3.0 / 2.0)) + 2 * pi) / 3);
//}
//
//float solve3()
//{
//    float p = 2.0;
//    float a = -8.0*p*c / pow(pi, 3);
//    float b = 0;
//    float c = 6.0*p*c / pi - 2;
//    float d = -p*pi - phi;
//    float alpha = b*c / (6.0*a*a) - b*b*b / (27.0*a*a*a) - d / (2.0*a);
//    float beta = c / (3.0*a) - b*b / (9.0*a*a);
//    return -b / (3 * a) + 2 * sqrt(-beta)*cos((acos(alpha / pow(-beta, 3.0 / 2.0)) - 2 * pi) / 3);
//}
//
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
    float gamma_i = solveTT(c, phi);
    float h = sin(gamma_i);
    float gamma_t = asin(h / mu_1);
    //float theta_t = asin(sin(theta_i / mu));
    float Fa = F(mu_1, mu_2, gamma_i, gamma_t);
    float F1_a = 1 - Fa;
    //float atten = pow(e, -(2 * absorptionr / cos(theta_t))*(1 + cos(2 * gamma_t)));
    float atten = 1.0f;
    return F1_a*F1_a*atten / abs((2.0 / mu_1) / sqrt(1 - h*h / (mu_1*mu_1)) - 2.0 / sqrt(1 - h*h));
}
//
//float N_TRT()
//{
//    float mu1 = 2.0*(mu - 1)*eccentricity*eccentricity - mu + 2.0;
//    float mu2 = 2.0*(mu - 1) / (eccentricity*eccentricity) - mu + 2.0;
//    mu = (mu1 + mu2 + cos(2 * phi_h)*(mu1 - mu2)) / 2.0;
//    mu_1 = abs(sqrt(mu*mu - sin(theta_d)*sin(theta_d)) / cos(theta_d));
//    mu_2 = abs(mu*mu*cos(theta_d) / sqrt(mu*mu - sin(theta_d)*sin(theta_d)));
//    c = asin(1.0 / mu_1);
//    float ans = 0.0;
//    int root = solve(2);
//    if (root == 1)
//    {
//        float gamma_i = 0.0;
//        if (testsolve(solve11(-2)))gamma_i = solve11(-2);
//        else if (testsolve(solve11(0)))gamma_i = solve11(0);
//        else if (testsolve(solve11(2)))gamma_i = solve11(2);
//        float h = sin(gamma_i);
//        float gamma_t = asin(h / mu_1);
//        float theta_t = asin(sin(theta_i / mu));
//        ans = (1 - F(mu_1, mu_2, gamma_i))*(1 - F(mu_1, mu_2, gamma_i))*F(1.0 / mu_1, 1.0 / mu_2, gamma_t)*pow(e, -(4 * absorption / cos(theta_t))*(1 + cos(2 * gamma_t))) / abs((2.0 / mu_1) / sqrt(1 - h*h / (mu_1*mu_1)) - 2.0 / sqrt(1 - h*h));
//        A = (1 - F(mu_1, mu_2, gamma_i))*(1 - F(mu_1, mu_2, gamma_i))*F(1.0 / mu_1, 1.0 / mu_2, gamma_t)*pow(e, -(4 * absorption / cos(theta_t))*(1 + cos(2 * gamma_t)));
//    }
//    else
//    {
//        float gamma_i = solve1();
//        float h = sin(gamma_i);
//        float gamma_t = asin(h / mu_1);
//        float theta_t = asin(sin(theta_i / mu));
//        ans += (1 - F(mu_1, mu_2, gamma_i))*(1 - F(mu_1, mu_2, gamma_i))*F(1.0 / mu_1, 1.0 / mu_2, gamma_t)*pow(e, -(4 * absorption / cos(theta_t))*(1 + cos(2 * gamma_t))) / abs((2.0 / mu_1) / sqrt(1 - h*h / (mu_1*mu_1)) - 2.0 / sqrt(1 - h*h));
//
//        gamma_i = solve2();
//        h = sin(gamma_i);
//        gamma_t = asin(h / mu_1);
//        ans += (1 - F(mu_1, mu_2, gamma_i))*(1 - F(mu_1, mu_2, gamma_i))*F(1.0 / mu_1, 1.0 / mu_2, gamma_t)*pow(e, -(4 * absorption / cos(theta_t))*(1 + cos(2 * gamma_t))) / abs((2.0 / mu_1) / sqrt(1 - h*h / (mu_1*mu_1)) - 2.0 / sqrt(1 - h*h));
//
//        gamma_i = solve3();
//        h = sin(gamma_i);
//        gamma_t = asin(h / mu_1);
//        ans += (1 - F(mu_1, mu_2, gamma_i))*(1 - F(mu_1, mu_2, gamma_i))*F(1.0 / mu_1, 1.0 / mu_2, gamma_t)*pow(e, -(4 * absorption / cos(theta_t))*(1 + cos(2 * gamma_t))) / abs((2.0 / mu_1) / sqrt(1 - h*h / (mu_1*mu_1)) - 2.0 / sqrt(1 - h*h));
//    }
//
//    float h_c = 0.0;
//    float phi_c = 0.0;
//    float delta_h = 0.0;
//    float t = 0.0;
//    if (mu_1 < 2)
//    {
//        h_c = (4 - mu_1*mu_1) / 3;
//        phi_c = 2.0*2.0*asin(h_c / mu_1) - 2.0*asin(h_c) + 2.0*pi;
//        phi_c = trinormal(phi_c);
//        delta_h = min(causticintensity, 2.0*sqrt(2.0*azimuthalwidth / abs(4 * h_c / (pow(mu_1, 3)*pow(1 - h_c*h_c / (mu_1*mu_1), 3.0 / 2.0)) - 2 * h_c / pow(1 - h_c*h_c, 3.0 / 2.0))));
//        t = 1;
//    }
//    else
//    {
//        phi_c = 0;
//        delta_h = causticintensity;
//        t = smoothstep(2, 2 + faderange, mu_1);
//    }
//    ans *= (1 - t*Gaussian(azimuthalwidth, phi - phi_c) / Gaussian(azimuthalwidth, 0));
//    ans *= (1 - t*Gaussian(azimuthalwidth, phi + phi_c) / Gaussian(azimuthalwidth, 0));
//    ans += t*glintscale*A*delta_h*(Gaussian(azimuthalwidth, phi - phi_c) + Gaussian(azimuthalwidth, phi + phi_c));
//
//    return ans;
//}

float M_R(float theta_h)
{
    return Gaussian(longwidth_R, (theta_h - longshift_R));
}

float M_TT(float theta_h)
{
    return Gaussian(longwidth_TT, theta_h - longshift_TT);
}
//
//float M_TRT()
//{
//    return Gaussian(longwidth_TRT, theta_h - longshift_TRT);
//}

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
    float mu_1 = abs(sqrt(mu*mu - sin(theta_i)*sin(theta_i)) / cos(theta_i));
    float mu_2 = abs(mu*mu*cos(theta_i) / sqrt(mu*mu - sin(theta_i)*sin(theta_i)));
    float c = asin(1.0 / mu_1);
    float m_r = M_R(theta_h);
    float n_r = N_R(phi, mu_1, mu_2);
    float m_tt = M_TT(theta_h);
    float n_tt = N_TT(phi, mu_1, mu_2, c);
    float cos2x = cos(theta_d)*cos(theta_d);
    return rgb * (n_r * m_r / cos2x /5 +
        m_tt*n_tt / cos2x /10);
    //    M_TRT()*N_TRT() / (cos(theta_d)*cos(theta_d)) * 250;
}

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 PS(PixelInputType input) : SV_TARGET
{
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
    float3 lightVec = {1/1.732, 1/1.732, -1/1.732};
    float theta_i = -acos(dot(-lightVec, vecU)) + pi / 2;
    float theta_r = -acos(dot(-viewVec, vecU)) + pi / 2;
    float phi_i = acos(dot(-lightVec, vecV));
    if (dot(-lightVec, vecW) < 0) phi_i = -phi_i;

    float phi_r = acos(dot(-viewVec, vecV));
    if (dot(-viewVec, vecW) < 0) phi_r = -phi_r;

    float3 diffuse = scattering(phi_i, phi_r, theta_i, theta_r, input.color.rgb);
    //float3 diffuse = input.color.rgb;

    /*if (input.color.x > 0.999 &&
        input.color.y > 0.999 &&
        input.color.z > 0.999)
        discard;*/

    // Set the bias value for fixing the floating point precision issues.
    bias = 0.001f;

    // Set the default output color to the ambient light value for all pixels.
    color = float3(0.05, 0.05, 0.05) * input.color.rgb; // ambient

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

        // Compare the depth of the shadow map value and the depth of the light to determine whether to shadow or to light this pixel.
        // If the light is in front of the object then light the pixel, if not then shadow this pixel since an object (occluder) is casting a shadow on it.
        // Determine the final diffuse color based on the diffuse color and the amount of light intensity.
        // 相当于light的方向是（1，1，-1）
        diffuse *= weight;
        color += diffuse;
        color += 0.8 * weight * input.color.rgb * (1 - abs(input.direction.x + input.direction.y - input.direction.z) / 1.732);
        // Saturate the final light color.
        color = saturate(color);
    }

    return float4(color, 1.0);
}