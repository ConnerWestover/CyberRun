cbuffer externalData : register(b0)
{
    matrix world;
};

struct VertexShaderInput
{
    float3 position		: POSITION;
    float2 uv			: TEXCOORD;
    float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
};

struct VertexToPixel
{
    float4 position		: SV_POSITION;
    float2 uv           : TEXCOORD0;
};

VertexToPixel main(VertexShaderInput input) {
	VertexToPixel output;

	output.position = mul(float4(input.position, 1.0f), world);
	output.uv = input.uv;

	return output;
}