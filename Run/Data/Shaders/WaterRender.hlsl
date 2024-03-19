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
	float3 worldPosition : POSITION;
	float4 worldTangent : TANGENT;
	float4 worldBinormal : BINORMAL;
	float4 worldNormal : NORMAL;
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
}
//------------------------------------------------------------------------------------------------
cbuffer ModelConstants : register(b3)
{
	float4x4 ModelMatrix;
	float4 ModelColor;
};

//------------------------------------------------------------------------------------------------
Texture2D dirtGrassDiffuseTexture : register(t0);

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
	return v2p;
}
//------------------------------------------------------------------------------------------------
float4 PixelMain(v2p_t input) : SV_Target0
{
	return input.color;
}
//------------------------------------------------------------------------------------------------
