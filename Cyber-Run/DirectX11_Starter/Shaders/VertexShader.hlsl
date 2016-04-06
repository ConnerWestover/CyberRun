
// Constant buffer for C++ data being passed in
cbuffer externalData : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
};

// Describes individual vertex data
struct VertexShaderInput
{
    float3 position		: POSITION;
    float2 uv			: TEXCOORD;
    float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
};

// Defines the output data of our vertex shader
struct VertexToPixel
{
    float4 position		: SV_POSITION;
    float3 normal       : NORMAL;
	float3 tangent		: TANGENT;
    float3 worldPos     : TEXCOORD0;
    float2 uv           : TEXCOORD1;
};

// The entry point for our vertex shader
VertexToPixel main(VertexShaderInput input)
{
    // Set up output
    VertexToPixel output;

    // Calculate output position
    matrix worldViewProj = mul(mul(world, view), projection);
    output.position = mul(float4(input.position, 1.0f), worldViewProj);

    // Take into account rotation (but not translation)
    // Most of the  world matrix
	output.normal = mul(input.normal, (float3x3)world);
	output.tangent = mul(input.tangent, (float3x3)world);

    // The world space position of the vertex
    output.worldPos = mul(float4(input.position, 1), world).xyz;

    // Just pass through
    output.uv = input.uv;

    return output;
}