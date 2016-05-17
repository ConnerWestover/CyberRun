struct VertexToPixel
{
    float4 position		: SV_POSITION;
    float2 uv           : TEXCOORD0;
};

Texture2D image				: register(t0);
SamplerState samplerState	: register(s0);

float4 main(VertexToPixel input) : SV_TARGET {
	float4 colour = image.Sample(samplerState, input.uv);

	return colour;
}