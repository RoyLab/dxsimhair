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
    float3 direction : DIRECTION0;
    float4 color: COLOR0;
    float4 lightViewPosition : TEXCOORD1;
};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
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

    // Set the bias value for fixing the floating point precision issues.
    bias = 0.001f;

    // Set the default output color to the ambient light value for all pixels.
    color = float4(0.1, 0.1, 0.1, 1.0) * diffuse; // ambient

    // Calculate the projected texture coordinates.
    projectTexCoord.x = input.lightViewPosition.x / input.lightViewPosition.w / 2.0f + 0.5f;
    projectTexCoord.y = -input.lightViewPosition.y / input.lightViewPosition.w / 2.0f + 0.5f;

    // Determine if the projected coordinates are in the 0 to 1 range.  If so then this pixel is in the view of the light.
    if ((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y))
    {
        // Sample the shadow map depth value from the depth texture using the sampler at the projected texture coordinate location.
        float dvs[4];
        int w = 1024, h = 704;
        float cw = w * projectTexCoord.x;
        float ch = h * projectTexCoord.y;

        depthValue = shaderTexture.Load(int3(int(round(cw)), int(round(ch)), 0)).r;
        //depthValue = shaderTexture.Sample(SampleTypeClamp, projectTexCoord).r;

        // Calculate the depth of the light.
        lightDepthValue = input.lightViewPosition.z / input.lightViewPosition.w;

        // Subtract the bias from the lightDepthValue.
        lightDepthValue = lightDepthValue - bias;

        // Compare the depth of the shadow map value and the depth of the light to determine whether to shadow or to light this pixel.
        // If the light is in front of the object then light the pixel, if not then shadow this pixel since an object (occluder) is casting a shadow on it.
        if (lightDepthValue < depthValue)
        {
            // Determine the final diffuse color based on the diffuse color and the amount of light intensity.
            color += diffuse;

            // Saturate the final light color.
            color = saturate(color);
        }
    }

    return color;
}