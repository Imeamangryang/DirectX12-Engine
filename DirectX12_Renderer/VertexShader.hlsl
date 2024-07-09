Texture2D<float4> heightmap : register(t0);

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float4 norm : NORMAL;
	float2 tex : TEXCOORD;
};

struct VS_INPUT
{
	float3 pos : POSITION;
	float3 norm : NORMAL;
};

cbuffer ConstantBuffer : register(b0)
{
	float4x4 world;
	float4x4 viewproj;
	float4 eye;
	int height;
	int width;
}

VS_OUTPUT VS(VS_INPUT input) {
	VS_OUTPUT output;

	output.pos = float4(input.pos.x, input.pos.y, input.pos.z, 1.0f);
	output.pos = mul(output.pos, viewproj);

	output.norm = float4(input.norm, 1.0f);

	float theta = atan2(output.norm.x, output.norm.z);
	float phi = acos(output.norm.y);

	output.tex.x = theta / (2.0f * 3.14f);
	output.tex.y = phi / 3.14f;

	return output;
}
