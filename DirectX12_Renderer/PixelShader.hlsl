Texture2D<float4> skymap : register(t0);
SamplerState smsampler : register(s0);

cbuffer ConstantBuffer : register(b0)
{
	float4x4 world;
	float4x4 viewproj;
	float4 eye;
	int height;
	int width;
}

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float4 norm : NORMAL;
    float3 tan : TANGENT;
    float2 tex : TEXCOORD;
};

float4 PS(VS_OUTPUT input) : SV_TARGET
{
	// 텍스처 좌표 계산
    float theta = atan2(input.norm.x, input.norm.z);
    float phi = acos(input.norm.y);
	
    float2 tex = float2(theta / 6.28318530718f, phi / 3.14159265359f);
	
	return skymap.Sample(smsampler, tex);
}