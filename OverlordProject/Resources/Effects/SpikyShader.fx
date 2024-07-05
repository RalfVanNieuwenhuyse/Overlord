//************
// VARIABLES *
//************
float4x4 m_MatrixWorldViewProj : WORLDVIEWPROJECTION;
float4x4 m_MatrixWorld : WORLD;
float3 m_LightDirection : DIRECTION
<
	string Object="TargetLight";
> = float3(0.577f, -0.577f, 0.577f);

float3 gColorDiffuse : COLOR = float3(1,1,1);

float gSpikeLength
<
	string UIWidget="Slider";
	float UIMin=0.0f;
	float UIMax=0.5f;
	float UIStep=0.0001f;
> = 0.2f;

RasterizerState FrontCulling 
{ 
	CullMode = FRONT; 
};

//**********
// STRUCTS *
//**********
struct VS_DATA
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
};

struct GS_DATA
{
    float4 Position : SV_POSITION;
    float3 Normal : NORMAL;
};

//****************
// VERTEX SHADER *
//****************
VS_DATA MainVS(VS_DATA vsData)
{
    return vsData;
}

//******************
// GEOMETRY SHADER *
//******************
void CreateVertex(inout TriangleStream<GS_DATA> triStream, float3 pos, float3 normal, float2 texCoord)
{
	//Step 1. Create a GS_DATA object
	GS_DATA gsData;

    //Step 2. Transform the position using the WVP Matrix and assign it to (GS_DATA object).Position (Keep in mind: float3 -> float4)
    gsData.Position = mul(float4(pos, 1), m_MatrixWorldViewProj);

    //Step 3. Transform the normal using the World Matrix and assign it to (GS_DATA object).Normal (Only Rotation, No translation!)
    gsData.Normal = mul(normal, (float3x3) m_MatrixWorld);

    //Step 4. Append (GS_DATA object) to the TriangleStream parameter (TriangleStream::Append(...))
    triStream.Append(gsData);
}

[maxvertexcount(9)]
void SpikeGenerator(triangle VS_DATA vertices[3], inout TriangleStream<GS_DATA> triStream)
{
	//Use these variable names
    float3 basePoint, top, left, right, spikeNormal;

	// Step 1: Calculate CENTER_POINT
	float3 centerPoint = (vertices[0].Position + vertices[1].Position + vertices[2].Position) / 3.0f;

	// Step 2: Calculate Face Normal (Original Triangle)
	spikeNormal = (vertices[0].Normal + vertices[1].Normal + vertices[2].Normal)/3.0f;

	// Step 3: Offset CENTER_POINT (use gSpikeLength)
	centerPoint = centerPoint + (spikeNormal * gSpikeLength);

	// Step 4 + 5: Calculate Individual Face Normals (Cross Product of Face Edges) & Create Vertices for every face

	// FACE 1
	float3 faceNormal1 = normalize(cross(centerPoint - vertices[0].Position, centerPoint - vertices[1].Position));
	basePoint = vertices[0].Position;
	top = centerPoint;
	left = vertices[1].Position;
	right = vertices[0].Position;

	CreateVertex(triStream, left, faceNormal1, float2(0, 0));
	CreateVertex(triStream,  top , faceNormal1, float2(0.5f, 1));
	CreateVertex(triStream, right , faceNormal1, float2(1, 0));

	// Restart Strip!
	triStream.RestartStrip();

	// FACE 2
	float3 faceNormal2 = normalize(cross(centerPoint - vertices[1].Position, centerPoint - vertices[2].Position));
	basePoint = vertices[1].Position;
	top = centerPoint;
	left = vertices[2].Position;
	right = vertices[1].Position;

	CreateVertex(triStream, left, faceNormal2, float2(0, 0));
	CreateVertex(triStream,  top , faceNormal2, float2(0.5f, 1));
	CreateVertex(triStream, right , faceNormal2, float2(1, 0));

	// Restart Strip!
	triStream.RestartStrip();

	// FACE 3
	float3 faceNormal3 = normalize(cross(centerPoint - vertices[0].Position, vertices[2].Position - centerPoint ));
	basePoint = vertices[2].Position;
	top = centerPoint;
	left = vertices[0].Position;
	right = vertices[2].Position;

	CreateVertex(triStream, top , faceNormal3, float2(0, 0));
	CreateVertex(triStream, right , faceNormal3, float2(0.5f, 1));
	CreateVertex(triStream, left , faceNormal3, float2(1, 0));

	triStream.RestartStrip();
	
}

//***************
// PIXEL SHADER *
//***************
float4 MainPS(GS_DATA input) : SV_TARGET
{
    input.Normal=normalize(input.Normal);
	float diffuseStrength = max(dot(normalize(m_LightDirection),-input.Normal),0.2f); 
	return float4(gColorDiffuse*diffuseStrength,1.0f);
}

//*************
// TECHNIQUES *
//*************
technique10 Default //FXComposer >> Rename to "technique10 Default"
{
    pass p0
    {
        SetRasterizerState(FrontCulling);
        SetVertexShader(CompileShader(vs_4_0, MainVS()));
        SetGeometryShader(CompileShader(gs_4_0, SpikeGenerator()));
        SetPixelShader(CompileShader(ps_4_0, MainPS()));
    }
}