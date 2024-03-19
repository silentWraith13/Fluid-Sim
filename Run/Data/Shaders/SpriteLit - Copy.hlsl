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
	float4 clipPosition : SV_Position;
	float4 worldPosition : POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
	float4 worldTangent : TANGENT;
	float4 worldBinormal : BINORMAL;
	float4 worldNormal : NORMAL;
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
Texture2D diffuseTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D specGlossEmitTexture : register(t2);

//------------------------------------------------------------------------------------------------
SamplerState samplerState : register(s0);

//------------------------------------------------------------------------------------------------
float GetZOffset(float4 worldPos)
{
	float pi = 3.14159265359f;
	float waveHeight = 100.f;
	float secondsPerWave = 2.0f;
	float timeScale = ( 2.0f *  pi )  / secondsPerWave; 
	float phaseX = (worldPos.x * 0.005);
	float phaseY = (worldPos.y * 0.01);
	float waveZ = waveHeight *  sin(Time * timeScale + phaseX + phaseY );

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

    v2p_t v2p;
	v2p.clipPosition = clipPosition;
	v2p.worldPosition = worldPosition;
    v2p.color = input.color;
    v2p.uv = input.uv;
	v2p.worldTangent = mul(ModelMatrix, float4(input.localTangent, 0));
	v2p.worldBinormal = mul(ModelMatrix, float4(input.localBinormal, 0));
	v2p.worldNormal = mul(ModelMatrix, float4(input.localNormal, 0));
    return v2p;
}
//------------------------------------------------------------------------------------------------
float4 PixelMain(v2p_t input) : SV_Target0
{
	float3 worldNormal;
	if (NormalMode == 0)
	{
		float3x3 TBNMatrix = float3x3(normalize(input.worldTangent.xyz), normalize(input.worldBinormal.xyz), normalize(input.worldNormal.xyz));
		float3 tangentNormal = 2.0f * normalTexture.Sample(samplerState, input.uv).rgb - 1.0f;
		worldNormal = mul(tangentNormal, TBNMatrix);
	}
	else
	{
		worldNormal = normalize(input.worldNormal.xyz);
	}
	float specularIntensity = 0.0f;
	float specularPower = 0.0f;
	if (SpecularMode == 0)
	{
		float3 specGlossEmit = specGlossEmitTexture.Sample(samplerState, input.uv).rgb;
		specularIntensity = specGlossEmit.r;
		specularPower = 31.0f * specGlossEmit.g + 1.0f;
	}
	else
	{
		specularIntensity = SpecularIntensity;
		specularPower = SpecularPower;
	}
	float3 worldViewDirection = normalize(WorldEyePosition - input.worldPosition.xyz);
	float3 worldHalfVector = normalize(-SunDirection + worldViewDirection);
	float ndotH = saturate(dot(worldNormal, worldHalfVector));
	float specular = pow(ndotH, specularPower) * specularIntensity;
	float ambient = AmbientIntensity;
	float directional = SunIntensity * saturate(dot(normalize(worldNormal), -SunDirection));
	float4 lightColor = saturate(float4((ambient + directional + specular).xxx, 1));

	bool isBlue = DoesVertexHaveOnlyBlueColor(input.color);
	float4 color;

	if (isBlue)
	{
		color = float4(0, 0.7f, 1, 1.f);
		color *= lightColor;
	}
	else
	{
		float4 textureColor = diffuseTexture.Sample(samplerState, input.uv);
		float4 vertexColor = input.color;
		float4 modelColor = ModelColor;
		color = lightColor * textureColor * vertexColor * modelColor;
	}

	clip(color.a - 0.01f); 
	return color;
}
//------------------------------------------------------------------------------------------------