float4x4 gWorld : WORLD;
float4x4 gWorldViewProj : WORLDVIEWPROJECTION; 
float3 gLightDirection = float3(-0.577f, -0.577f, 0.577f);

Texture2D gDiffuseMap;
SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;// or Mirror or Clamp or Border
    AddressV = Wrap;// or Mirror or Clamp or Border
};

RasterizerState Solid
{
	FillMode = SOLID;
	CullMode = FRONT;
};

struct VS_INPUT{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
	float4 blendIndices : BLENDINDICES;
	float4 blendWeights : BLENDWEIGHTS;
};
struct VS_OUTPUT{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
};

DepthStencilState EnableDepth
{
	DepthEnable = TRUE;
	DepthWriteMask = ALL;
};

RasterizerState NoCulling
{
	CullMode = NONE;
};

BlendState NoBlending
{
	BlendEnable[0] = FALSE;
};

float4x4 gBones[70];

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input){

    VS_OUTPUT output;

    // Step 1: Convert position into float4 and multiply with matWorldViewProj
    output.pos = mul(float4(input.pos, 1.0f), gWorldViewProj);

    // Step 2: Rotate the normal: NO TRANSLATION
    //         This is achieved by clipping the 4x4 to a 3x3 matrix,
    //         thus removing the position row of the matrix.
    output.normal = normalize(mul(input.normal, (float3x3)gWorld));

    // Step 3: Transform position and normal for each bone
    float4 transformedPosition = float4(0, 0, 0, 0);
    float3 transformedNormal = float3(0, 0, 0);

    for (int i = 0; i < 4; i++) 
	{
		int boneIndex = input.blendIndices[i];
		float boneWeight = input.blendWeights[i];

		if (boneIndex >= 0 && boneWeight > 0.0f) {
			float4x4 boneTransform = gBones[boneIndex];

			float3 boneSpacePosition = mul(float4(input.pos,1.0f), (float4x4)boneTransform);
			float3 boneSpaceNormal = mul(input.normal, (float3x3)boneTransform);

			transformedPosition += float4(boneSpacePosition * boneWeight, 0.0f);
			transformedNormal += boneSpaceNormal * boneWeight;
		}		 
	}

    // Step 4: Ensure that the w-component of the transformedPosition is 1
    transformedPosition.w = 1.0f;
	

    // Step 5: Transform the transformedPosition to Clipping Space (WVP Matrix)
    output.pos = mul(transformedPosition, gWorldViewProj);

    // Step 6: Transform the transformedNormal to World Space (World Matrix) - (Rotation only!)
    output.normal = normalize(mul(transformedNormal, (float3x3)gWorld));

    // Store the transformed texture coordinates
    output.texCoord = input.texCoord;

    // Return the output
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_TARGET{

	float4 diffuseColor = gDiffuseMap.Sample( samLinear,input.texCoord );
	float3 color_rgb= diffuseColor.rgb;
	float color_a = diffuseColor.a;
	
	//HalfLambert Diffuse :)
	float diffuseStrength = dot(input.normal, -gLightDirection);
	diffuseStrength = diffuseStrength * 0.5 + 0.5;
	diffuseStrength = saturate(diffuseStrength);
	color_rgb = color_rgb * diffuseStrength;

	return float4( color_rgb , color_a );
}

//--------------------------------------------------------------------------------------
// Technique
//--------------------------------------------------------------------------------------
technique11 Default
{
    pass P0
    {
		SetRasterizerState(NoCulling);
		SetDepthStencilState(EnableDepth, 0);
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);

		SetVertexShader( CompileShader( vs_4_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}

