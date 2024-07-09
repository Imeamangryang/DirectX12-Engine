Texture2D<float4> displacementmap : register(t0);
SamplerState dmsampler : register(s0);

struct VS_OUTPUT
{
	float3 pos : POSITION;
	float3 norm : NORMAL;
	float3 tan : TANGENT;
};

struct VS_INPUT
{
	float3 pos : POSITION;
	float3 norm : NORMAL;
	float3 tan : TANGENT;
};

struct LightData {
	float4 pos;
	float4 amb;
	float4 dif;
	float4 spec;
	float3 att;
	float rng;
	float3 dir;
	float sexp;
};

cbuffer ConstantBuffer : register(b0)
{
	float4x4 viewproj;
	float4 eye;
	LightData light;
	int height;
	int width;
}

VS_OUTPUT VSTes(VS_INPUT input) {
	VS_OUTPUT output;

	output.pos = input.pos;
	output.norm = input.norm;
	output.tan = input.tan;

	return output;
}