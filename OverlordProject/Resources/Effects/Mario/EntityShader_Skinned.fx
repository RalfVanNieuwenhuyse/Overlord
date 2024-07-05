float4x4 gWorld : WORLD;
float4x4 gWorldViewProj : WORLDVIEWPROJECTION; 
float3 gLightDirection = float3(-0.577f, -0.577f, 0.577f);
float4x4 gBones[70];


Texture2D gDiffuseMap;
Texture2D gOpacityMap;
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
	float4 color : COLOR;
	
	float4 blendIndices : BLENDINDICES;
	float4 blendWeights : BLENDWEIGHTS;
};
struct VS_OUTPUT{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
	float4 color : COLOR;
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

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input){

	VS_OUTPUT output;
    
    output.pos = mul(float4(input.pos, 1.0f), gWorldViewProj);
    output.normal = normalize(mul(input.normal, (float3x3)gWorld));

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

    transformedPosition.w = 1.0f;	
    output.pos = mul(transformedPosition, gWorldViewProj);
    output.normal = normalize(mul(transformedNormal, (float3x3)gWorld));
	output.color = input.color;
    output.texCoord = input.texCoord;

    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_TARGET{


	float oppacity = gOpacityMap.Sample( samLinear,input.texCoord ).x;

	float4 color;
    if(oppacity == 1)
    {
    	color = gDiffuseMap.Sample( samLinear,input.texCoord );
    }
	else
	{
		color = input.color;
	}

	float3 color_rgb= color.rgb;
	float color_a = color.a;
	
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
