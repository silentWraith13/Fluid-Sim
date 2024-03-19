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
cbuffer TimeCB : register(b0)
{
	float4 cameraWorldPos;
	float Time;
	float fogStartDistance;
	float fogEndDistance;
	float fogMaxAlpha;
	int cull;
}
//------------------------------------------------------------------------------------------------
cbuffer LightConstants : register(b1)
{
	float3 SunDirection;
	float SunIntensity;
	float AmbientIntensity;
	float3 WorldEyePosition;
	int NormalMode;
};
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
cbuffer ClippingPlane : register(b4)
{
	float4 clippingPlane;
}
//------------------------------------------------------------------------------------------------
Texture2D dirtGrassDiffuseTexture : register(t0);
Texture2D dirtGrassNormalTexture : register(t1);

Texture2D snowDiffuseTexture : register(t2);
Texture2D snowNormalTexture : register(t3);

Texture2D waterDiffuseTexture : register(t4);
Texture2D waterNormalTexture : register(t5);
//------------------------------------------------------------------------------------------------
SamplerState diffuseSampler : register(s0);

//------------------------------------------------------------------------------------------------
float GetZOffset(float4 worldPos)
{
	float pi = 3.14159265359f;
	float waveHeight = 4.f;
	float secondsPerWave = 4.0f;
	float timeScale = (2.0f * pi) / secondsPerWave;
	float phaseX = (worldPos.x * 0.1);
	float phaseY = (worldPos.y * 0.1);
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
	v2p.color = input.color;
	v2p.uv = input.uv;
	v2p.worldPosition = worldPosition.xyz;
	v2p.worldTangent = mul(ModelMatrix, float4(input.localTangent, 0));
	v2p.worldBinormal = mul(ModelMatrix, float4(input.localBinormal, 0));
	v2p.worldNormal = mul(ModelMatrix, float4(input.localNormal, 0));
	return v2p;
}
//------------------------------------------------------------------------------------------------
float RangeMap(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	if (inStart == inEnd)
	{
		return outStart + (outEnd - outStart) * 0.5;
	}
	float normalizedValue = (inValue - inStart) / (inEnd - inStart);

	return outStart + (outEnd - outStart) * normalizedValue;
}
//------------------------------------------------------------------------------------------------
float GetSnownessForWorldPosition(float3 worldPos)
{
	float snowness = RangeMap(worldPos.z, 100, 1000, 0, 1);
	snowness = saturate(snowness);
	return snowness;
}
//------------------------------------------------------------------------------------------------
float3 CalculateWorldNormal(float3 tangentNormal, float3x3 TBNMatrix, float3 worldNormal)
{
	return NormalMode == 1 ? mul(tangentNormal, TBNMatrix) : worldNormal;
}
//------------------------------------------------------------------------------------------------
float4 PixelMain(v2p_t input) : SV_Target0
{
	float side = dot(input.worldPosition, clippingPlane.xyz) + clippingPlane.w;

	if ( cull == 0)
	{
		if ( side > 0)
		{
			discard;
		}
	}
	else if (cull == 1 )
	{
		if (side < 0)
		{
			discard;
		}
	}
	

	 float4 skyColor = float4(0.53, 0.81, 0.92, 1.0);
	float2 animatedUV = input.uv + float2(0.1f, 0.1f) * Time;
	float4 waterDiffuseColor = waterDiffuseTexture.Sample(diffuseSampler, animatedUV);
	float4 grassDiffuseColor = dirtGrassDiffuseTexture.Sample(diffuseSampler, input.uv);
	float4 snowColor = snowDiffuseTexture.Sample(diffuseSampler, input.uv);

	float3 waterNormal = 2.0f * waterNormalTexture.Sample(diffuseSampler, animatedUV).rgb - 1.0f; //[-1,1]
	float3 grassNormal = 2.0f * dirtGrassNormalTexture.Sample(diffuseSampler, input.uv).rgb - 1.0f;

	bool isBlue = DoesVertexHaveOnlyBlueColor(input.color);
	float3 worldNormal = normalize(input.worldNormal.xyz);
	float3x3 TBNMatrix = float3x3(normalize(input.worldTangent.xyz), normalize(input.worldBinormal.xyz), worldNormal);

	float3 normal = isBlue ? waterNormal : grassNormal;
	worldNormal = NormalMode == 1 ? mul(normal, TBNMatrix) : worldNormal;

	float ambient = AmbientIntensity;
	float directionalLight = SunIntensity * saturate(dot(normalize(worldNormal), -SunDirection));
	float4 lightColor = saturate(float4((ambient + directionalLight).xxx, 1));
	
	float snowness = GetSnownessForWorldPosition(input.worldPosition);
	float4 blendedColor = lerp(grassDiffuseColor, snowColor, snowness);
	
	float distanceFromCamera = length(input.worldPosition.xyz - cameraWorldPos.xyz);
	float fogFraction = fogMaxAlpha * saturate((distanceFromCamera - fogStartDistance) / (fogEndDistance - fogStartDistance));
	
	float4 finalColor;
	float4 outputRGB;
	float outputAlpha;
	float4 outputRGBA;
	if (isBlue)
	{
		finalColor = waterDiffuseColor * lightColor;
		outputRGB = lerp(skyColor, finalColor, fogFraction * fogMaxAlpha);
		outputAlpha = saturate(finalColor.a + fogFraction);
		outputRGBA = float4(outputRGB.xyz, outputAlpha);
	}
	else
	{
		finalColor = blendedColor * input.color * ModelColor * lightColor;
		 outputRGB = lerp(skyColor, finalColor, fogFraction * fogMaxAlpha);
		outputAlpha = saturate(finalColor.a + fogFraction);
		 outputRGBA = float4(outputRGB.xyz, outputAlpha);
	}
	

	return finalColor;
}
//------------------------------------------------------------------------------------------------
