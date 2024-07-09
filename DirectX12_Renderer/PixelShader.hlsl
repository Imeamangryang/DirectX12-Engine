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
	float2 tex : TEXCOORD;
};

float4 PS(VS_OUTPUT input) : SV_TARGET
{
	return skymap.Sample(smsampler, input.tex);
}