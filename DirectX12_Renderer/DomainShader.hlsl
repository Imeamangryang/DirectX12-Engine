Texture2D<float4> displacementmap : register(t0);
SamplerState dmsampler : register(s0);

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
    float4x4 world;
	float4x4 viewproj;
	float4 eye;
	LightData light;
	int height;
	int width;
}

struct DS_OUTPUT
{
	float4 pos : SV_POSITION;
	float4 norm : NORMAL;
	float3 tan : TANGENT;
	float2 tex : TEXCOORD;
};

struct HS_CONTROL_POINT_OUTPUT
{
	float3 pos : POSITION;
	float3 norm : NORMAL;
	float3 tan : TANGENT;
};

struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[3]	: SV_TessFactor; 
	float InsideTessFactor : SV_InsideTessFactor;
};

#define NUM_CONTROL_POINTS 3

[domain("tri")]
DS_OUTPUT DS(
	HS_CONSTANT_DATA_OUTPUT input,
	float3 domain : SV_DomainLocation,
	const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> patch)
{
	DS_OUTPUT output;

	output.pos = float4(patch[0].pos.xyz * domain.x + patch[1].pos.xyz * domain.y + patch[2].pos.xyz * domain.z, 1);

	output.norm = float4(patch[0].norm.xyz * domain.x + patch[1].norm.xyz * domain.y + patch[2].norm.xyz * domain.z, 1.0f);
	output.tan = float3(patch[0].tan.xyz * domain.x + patch[1].tan.xyz * domain.y + patch[2].tan.xyz * domain.z);

	//float scale = height / 10;
	float scale = height / 150;

	float theta = atan2(output.norm.z, output.norm.x);
    if (theta < 0.0f)
        theta += 2.0f * 3.14159265359f;
	float phi = acos(output.norm.y);

	output.tex.x = theta / (2.0f * 3.14159265359f);
	output.tex.y = phi / 3.14159265359f;

	float hei = scale * displacementmap.SampleLevel(dmsampler, output.tex.xy, 0).r;

	output.pos.xyz += output.norm * hei;

	output.pos = float4(output.pos.xyz, 1.0f);

	float2 b = output.tex.xy + float2(0.0f, -0.3f / height);
	float2 c = output.tex.xy + float2(0.3f / width, -0.3f / height);
	float2 d = output.tex.xy + float2(0.3f / width, 0.0f);
	float2 e = output.tex.xy + float2(0.3f / width, 0.3f / height);
	float2 f = output.tex.xy + float2(0.0f, 0.3f / height);
	float2 g = output.tex.xy + float2(-0.3f / width, 0.3f / height);
	float2 h = output.tex.xy + float2(-0.3f / width, 0.0f);
	float2 i = output.tex.xy + float2(-0.3f / width, -0.3f / height);

	float zb = displacementmap.SampleLevel(dmsampler, b, 0) * scale;
	float zc = displacementmap.SampleLevel(dmsampler, c, 0) * scale;
	float zd = displacementmap.SampleLevel(dmsampler, d, 0) * scale;
	float ze = displacementmap.SampleLevel(dmsampler, e, 0) * scale;
	float zf = displacementmap.SampleLevel(dmsampler, f, 0) * scale;
	float zg = displacementmap.SampleLevel(dmsampler, g, 0) * scale;
	float zh = displacementmap.SampleLevel(dmsampler, h, 0) * scale;
	float zi = displacementmap.SampleLevel(dmsampler, i, 0) * scale;

	float x = zg + 2 * zh + zi - zc - 2 * zd - ze;
	float y = 2 * zb + zc + zi - ze - 2 * zf - zg;
	float z = 8.0f;

	float3 normal = normalize(float3(x, y, z));

	float3 N = normalize(patch[0].norm.xyz * domain.x + patch[1].norm.xyz * domain.y + patch[2].norm.xyz * domain.z);
	float3 T = normalize(patch[0].tan.xyz * domain.x + patch[1].tan.xyz * domain.y + patch[2].tan.xyz * domain.z);
	float3 B = normalize(cross(N, T));

	float3x3 TBN = float3x3(T, B, N);

	TBN = transpose(TBN);

	normal = mul(TBN, normal);

	//output.norm = float4(normal, 1.0f);

	// MVP º¯È¯
	output.pos = mul(world, output.pos);
	output.pos = mul(output.pos, viewproj);

	return output;
}

