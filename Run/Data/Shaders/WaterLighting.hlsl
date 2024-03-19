//------------------------------------------------------------------------------------------------
struct vs_input_t
{
	float3 localPosition : POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
	float3 localTangent : TANGENT;
	float3 localBinormal : BINORMAL;
	float3 localNormal : NORMAL;
};

//------------------------------------------------------------------------------------------------
struct v2p_t
{
	float4 position : SV_Position;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
	float3 worldPosition : WORLDPOS; 
	float4 worldTangent : TANGENT;
	float4 worldBinormal : BINORMAL;
	float4 worldNormal : NORMAL;
	float4 clipPosition : CLIPPOS; 
};
//------------------------------------------------------------------------------------------------
cbuffer CameraConstants : register(b2)
{
	float4x4 ProjectionMatrix;
	float4x4 ViewMatrix;
};
//------------------------------------------------------------------------------------------------
cbuffer ClippingPlane : register(b0)
{
	float4 clippingPlane;
	float time;
	float3 cameraPos;
}
//------------------------------------------------------------------------------------------------
cbuffer ModelConstants : register(b3)
{
	float4x4 ModelMatrix;
	float4 ModelColor;
};
//------------------------------------------------------------------------------------------------
cbuffer LightConstants : register(b1)
{
	float3 SunDirection;
	float SunIntensity;
	float AmbientIntensity;
	float3 WorldEyePosition;
	int NormalMode;
	int SpecularMode;
	float SpecularIntensity;
	float SpecularPower;
};
//------------------------------------------------------------------------------------------------
Texture2D reflectionRenderTexture : register(t0);
Texture2D refractionRenderTexture : register(t1);
Texture2D waterDudvTexture : register(t2);
Texture2D waterNormalTexture : register(t3);
//------------------------------------------------------------------------------------------------
SamplerState diffuseSampler : register(s0);

//------------------------------------------------------------------------------------------------
v2p_t VertexMain(vs_input_t input)
{
	float4 localPosition = float4(input.localPosition, 1);
	float4 worldPosition = mul(ModelMatrix, localPosition);
	float4 viewPosition = mul(ViewMatrix, worldPosition);
	float4 clipPosition = mul(ProjectionMatrix, viewPosition);
	float4 localNormal = float4(input.localNormal, 0);
	float4 worldNormal = mul(ModelMatrix, localNormal);

	v2p_t v2p;
	v2p.position = clipPosition;
	v2p.color = input.color;
	v2p.uv = input.uv;
	v2p.worldPosition = worldPosition.xyz;
	v2p.worldTangent = mul(ModelMatrix, float4(input.localTangent, 0));
	v2p.worldBinormal = mul(ModelMatrix, float4(input.localBinormal, 0));
	v2p.worldNormal = mul(ModelMatrix, float4(input.localNormal, 0));
	v2p.clipPosition = clipPosition;
	return v2p;
}
//------------------------------------------------------------------------------------------------
float4 PixelMain(v2p_t input) : SV_Target0
{
	float waveStrength = 0.005;
	float moveFactor = 0.02 * time;
	float ndcX = (input.clipPosition.x / input.clipPosition.w) / 2 + 0.5;
	float ndcY = 0.5 - (input.clipPosition.y / input.clipPosition.w) / 2;
	float2 ndc = float2(ndcX, ndcY);
	float2 reflectUV = float2(ndc.x, -ndc.y);
	float2 refractUV = float2(ndc.x, ndc.y); 
	float2 distortedTexCoords = waterDudvTexture.Sample(diffuseSampler, float2(input.uv.x + moveFactor, input.uv.y)).rg * 0.1;
	distortedTexCoords = input.uv + float2(distortedTexCoords.x, distortedTexCoords.y + moveFactor);
	float2 totalDistortion = (waterDudvTexture.Sample(diffuseSampler, distortedTexCoords).rg * 2.0 - 1.0) * waveStrength;
	refractUV += totalDistortion;
	reflectUV += totalDistortion;

	float4 normalMapColor = waterNormalTexture.Sample(diffuseSampler, distortedTexCoords);
	float3 normal = float3(normalMapColor.r * 2 - 1, normalMapColor.b * 2 - 1, normalMapColor.g * 2 - 1);
	normal = normalize(normal);
	
	float3 worldViewDirection = normalize(WorldEyePosition - input.worldPosition.xyz);
	float3 worldHalfVector = normalize(-SunDirection + worldViewDirection);
	float ndotH = saturate(dot(normal, worldHalfVector));
	float specular = pow(ndotH, SpecularPower) * SpecularIntensity;

	
	float refractiveFactor = dot(worldViewDirection, normal);
	refractiveFactor = pow(abs(refractiveFactor), 0.5);

	float4 reflectTextureColor = reflectionRenderTexture.Sample(diffuseSampler, reflectUV);
	float4 refractionTextureColor = refractionRenderTexture.Sample(diffuseSampler, refractUV);

	float ambient = AmbientIntensity;
	float directionalLight = SunIntensity * saturate(dot(normalize(normal), -SunDirection));
	float4 lightColor = saturate(float4((ambient + directionalLight + specular).xxx, 1));
	
	float4 color = lerp(reflectTextureColor, refractionTextureColor, refractiveFactor) * lightColor;
	color = lerp(color, float4(0, 0.3, 0.5, 1), 0.1);
	return color;
}
//------------------------------------------------------------------------------------------------
