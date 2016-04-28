
cbuffer LightData : register(b0)
{
	// Direction light stuff
	float4 DirLightColor;
	float3 DirLightDirection;

	// Point light
	float4 PointLightColor;
	float3 PointLightPosition;

	// Camera position
	float3 CameraPosition;

	float pixelWidth;
	float pixelHeight;
	float blurAmount;
	float bloomAmount;
}


// Defines the input to this pixel shader
// - Should match the output of our corresponding vertex shader
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 normal       : NORMAL;
	float3 tangent		: TANGENT;
	float3 worldPos     : TEXCOORD0;
	float2 uv           : TEXCOORD1;
};

// Textures and such
Texture2D diffuse		: register(t0);
Texture2D normalMap		: register(t1);
TextureCube skyTexture	: register(t2);
SamplerState trilinear	: register(s0);
Texture2D pixels		: register(t0);


// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
	// Re-normalize interpolated normal
	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent);

	// Handle normal mapping -----------------------------------
	float3 normalFromMap = normalMap.Sample(trilinear, input.uv).rgb;
	
	// Unpack the normal
	normalFromMap = normalFromMap * 2 - 1;

	// Calculate the TBN matrix to go from tangent-space to world-space
	float3 N = input.normal;
	float3 T = normalize(input.tangent - N * dot(input.tangent, N));
	float3 B = cross(T, N);

	float3x3 TBN = float3x3(T, B, N);

	// Adjust the normal that is used by the rest of the lighting calcs
	input.normal = normalize(mul(normalFromMap, TBN));
	

	// Directional light ---------------------------------------

	// Light direction
	float3 lightDir = normalize(DirLightDirection);

	// Basic diffuse calculation (n dot l)
	// We need the direction from the surface TO the light
	float dirNdotL = saturate(dot(input.normal, -lightDir));

	// Point light --------------------------------------------

	// Get direction to the point light
	float3 dirToPointLight = normalize(PointLightPosition - input.worldPos);
	float pointNdotL = saturate(dot(input.normal, dirToPointLight));

	// Point light specular ----------------------------------
	float3 dirToCamera = normalize(CameraPosition - input.worldPos);
	float3 refl = reflect(-dirToPointLight, input.normal);
	float spec = pow(max(dot(refl, dirToCamera), 0), 64.0f);

	// Grab the diffuse color
	float4 diffuseColor = diffuse.Sample(trilinear, input.uv);

	// Get the reflection color
	float4 reflectionColor = skyTexture.Sample(
		trilinear,
		reflect(-dirToCamera, input.normal));

	// Combine lights
	float4 surfaceColor = (PointLightColor * pointNdotL * diffuseColor)* (PointLightColor.w*10.0f) + (DirLightColor * dirNdotL * diffuseColor)* (DirLightColor.w*10.0f) + float4(spec.xxx, 1);

	if (surfaceColor.x + surfaceColor.y + surfaceColor.z > 1.5f) {
		surfaceColor.x += bloomAmount;
		surfaceColor.y += bloomAmount;
		surfaceColor.z += bloomAmount;

	}

	return lerp(reflectionColor, surfaceColor, 1.0f);
}