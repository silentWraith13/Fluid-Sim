//------------------------------------------------------------------------------------------------
struct vs_input_t
{
	float3 localPosition : POSITION;
	float3 localNormal : NORMAL;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

//------------------------------------------------------------------------------------------------
struct v2p_t
{
	float4 position : SV_Position;
	float3 normal : NORMAL;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};
//------------------------------------------------------------------------------------------------
cbuffer TimeCB : register(b0)
{
	float Time;
	float3 padding;
}

//------------------------------------------------------------------------------------------------
cbuffer CameraConstants : register(b2)
{
	float4x4 ProjectionMatrix;
	float4x4 ViewMatrix;
};

//------------------------------------------------------------------------------------------------
cbuffer ModelConstants : register(b3)
{
	float4x4 ModelMatrix;
	float4 ModelColor;
};

//------------------------------------------------------------------------------------------------
Texture2D diffuseTexture : register(t0);

//------------------------------------------------------------------------------------------------
SamplerState diffuseSampler : register(s0);

//------------------------------------------------------------------------------------------------
float GetZOffset(float4 worldPos)
{
	float pi = 3.14159265359f;
	float waveHeight = 100.f;
	float secondsPerWave = 2.0f;
	float timeScale = (2.0f * pi) / secondsPerWave;
	float phaseX = (worldPos.x * 0.005);
	float phaseY = (worldPos.y * 0.01);
	float waveZ = waveHeight * sin(Time * timeScale + phaseX + phaseY);

	return waveZ;
}
//------------------------------------------------------------------------------------------------
bool DoesVertexHaveOnlyBlueColor(float4 inputColor)
{
	float4 blueColor = float4(0, 0, 1, 1);
	float threshold = 0.05f;

	if (abs(inputColor.r - blueColor.r) < threshold &&
		abs(inputColor.g - blueColor.g) < threshold &&
		abs(inputColor.b - blueColor.b) < threshold &&
		abs(inputColor.a - blueColor.a) < threshold)
	{
		return true;
	}
	else
	{
		return false;
	}
}
//------------------------------------------------------------------------------------------------
v2p_t VertexMain(vs_input_t input)
{
	float4 localPosition = float4(input.localPosition, 1);
	float4 worldPosition = mul(ModelMatrix, localPosition);

	bool isBlue = DoesVertexHaveOnlyBlueColor(input.color);

	if (isBlue)
	{
		float zOffset = GetZOffset(worldPosition);
		worldPosition.z += zOffset;
	}

	float4 viewPosition = mul(ViewMatrix, worldPosition);
	float4 clipPosition = mul(ProjectionMatrix, viewPosition);
	float4 localNormal = float4(input.localNormal, 0);
	float4 worldNormal = mul(ModelMatrix, localNormal);

	v2p_t v2p;
	v2p.position = clipPosition;
	v2p.normal = worldNormal.xyz;
	v2p.color = input.color;
	v2p.uv = input.uv;
	return v2p;
}
//------------------------------------------------------------------------------------------------
float4 PixelMain(v2p_t input) : SV_Target0
{
	bool isBlue = DoesVertexHaveOnlyBlueColor(input.color);
	float4 color;

	if (isBlue)
	{
		color = float4(0, 0.7f, 1, 0.5f);
	}
	else
	{
		float4 textureColor = diffuseTexture.Sample(diffuseSampler, input.uv);
		float4 vertexColor = input.color;
		float4 modelColor = ModelColor;
		color = textureColor * vertexColor * modelColor;
	}

	clip(color.a - 0.01f);
	return color;
}
//------------------------------------------------------------------------------------------------