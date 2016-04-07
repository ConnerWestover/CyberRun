
cbuffer PerFrame : register(b0)
{
	float2 g_offset[NUM_SAMPLES];
	float4 g_weight[NUM_SAMPLES];
};

// Shader
float4 main(VsOutput a) : SV_TARGET
{
	float4 sample = 0;
float4 color = 0;
float2 position = 0;

[unroll]
for (int i = 0; i < NUM_SAMPLES; ++i)
{
	// Sample from adjacent points
	position = a.uv + g_offset[i];
	color = g_skybox.Sample(g_pointSampler, position);
	sample += g_weight[i] * color;
}

return sample;
}