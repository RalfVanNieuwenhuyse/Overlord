//=============================================================================
//// Shader uses position and texture
//=============================================================================
SamplerState samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Mirror;
    AddressV = Mirror;
};

Texture2D gTexture;

/// Create Depth Stencil State (ENABLE DEPTH WRITING)
/// Create Rasterizer State (Backface culling) 


//IN/OUT STRUCTS
//--------------
struct VS_INPUT
{
    float3 Position : POSITION;
	float2 TexCoord : TEXCOORD0;

};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD1;
};


//VERTEX SHADER
//-------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	// Set the Position
	output.Position = float4(input.Position,1);
	// Set the TexCoord
	output.TexCoord = input.TexCoord;
	
	return output;
}


//PIXEL SHADER
//------------
float4 PS(PS_INPUT input): SV_Target
{
    // Step 1: Find the dimensions of the texture	
    uint2 textureDimensions;
	uint numbreOfLevels;
	gTexture.GetDimensions(0,textureDimensions.x,textureDimensions.y,numbreOfLevels);

    // Step 2: Calculate dx and dy (UV space for 1 pixel)
    float dx = 1.0f / textureDimensions.x;
    float dy = 1.0f / textureDimensions.y;

    // Step 3: Create a double for loop (5 iterations each)
    const int numPasses = 5;
    float4 finalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);	
    for (int i = -2; i <= 2; i++)
    {
        for (int j = -2; j <= 2; j++)
        {
            // Calculate the offset in each direction
            float2 offset = float2(i * dx * 2.f, j * dy * 2.f);

            // Do a texture lookup using the previously calculated UV coordinates + offset
            float2 uv = input.TexCoord + offset;
            float4 texColor = gTexture.Sample(samPoint, uv);
			
            // Add to the final color
            finalColor += texColor;
        }
    }

    // Step 4: Divide the final color by the number of passes (in this case, 5*5)
    finalColor /= 25;
	finalColor.w = 1.0f;
    // Step 5: Return the final color
    return finalColor;
}


//TECHNIQUE
//---------
technique11 Blur
{
    pass P0
    {
		// Set states...
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}