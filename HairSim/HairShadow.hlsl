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
    int    Sequence :   SEQ;
    float3 Position     : POSITION;
    float3 Color        : COLOR;
    float3 Direction    : DIRECTION;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float  Sequence : SEQ0;
    float3 direction : DIRECTION0;
    float4 color: COLOR0;
    float4 lightViewPosition : TEXCOORD1;
};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////


float3 GetColour(float v)
{
   float3 c = {1.0,1.0,1.0}; // white
   float dv;

   if (v < 0.0)
      v = 0.0;
   if (v > 1.0)
      v = 1.0;
   dv = 1.0;

   if (v < (0.25 * dv)) {
      c.r = 0;
      c.g = 4 * (v) / dv;
   } else if (v < (0.5 * dv)) {
      c.r = 0;
      c.b = 1 + 4 * (0.25 * dv - v) / dv;
   } else if (v < (0.75 * dv)) {
      c.r = 4 * (v - 0.5 * dv) / dv;
      c.b = 0;
   } else {
      c.g = 1 + 4 * (0.75 * dv - v) / dv;
      c.b = 0;
   }

   return(c);
}

PixelInputType VS(VertexInputType input)
{
    PixelInputType output;
	float4 worldPosition;
    
    
	// Change the position vector to be 4 units for proper matrix calculations.
    float4 pos = float4(input.Position, 1.0);

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(pos, g_mViewProjection);
    
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

    output.Sequence = float(input.Sequence);

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



////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 PS(PixelInputType input) : SV_TARGET
{
    float bias;
    float4 color;
    float2 projectTexCoord;
    float depthValue;
    float lightDepthValue;
    float4 diffuse = input.color;
	
	if (input.color.x > 0.999 &&
		input.color.y > 0.999 &&
		input.color.z > 0.999)
		discard;

    // Set the bias value for fixing the floating point precision issues.
    bias = 0.001f;

    // Set the default output color to the ambient light value for all pixels.
    color = float4(0.2, 0.2, 0.2, 1.0) * diffuse; // ambient

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
        color += diffuse * weight * abs(input.direction.x + input.direction.y+ input.direction.z) / 2.0;

        // Saturate the final light color.
        color = saturate(color);
    }

    return color;
}