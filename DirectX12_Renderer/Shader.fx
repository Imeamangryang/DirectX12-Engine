Texture2D<float4> heightmap : register(t0);
SamplerState hmsampler : register(s0);

cbuffer ConstantBuffer : register(b0)
{
	float4x4 viewproj;
	float4 eye;
	int height;
	int width;
}

// shader 1

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD;
};


float4 PS(VS_OUTPUT input) : SV_TARGET
{
	return heightmap.Sample(hmsampler, input.tex);
}

VS_OUTPUT VS(uint input : SV_VERTEXID) 
{
	VS_OUTPUT output;

	output.pos = float4(float2((input << 1) & 2, input == 0) * float2(2.0f, -4.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
	output.tex = float2((output.pos.x + 1) / 2, (output.pos.y + 1) / 2);

	return output;
}


// shader 2

struct VS_OUTPUT2
{
	float4 pos : SV_POSITION;
	float4 worldpos : POSITION;
	float4 norm : NORMAL;
	float4 tex : TEXCOORD;
};

float4 PS2(VS_OUTPUT2 input) : SV_TARGET
{
	float4 light = normalize(float4(1.0f, 1.0f, -1.0f, 1.0f));
	float diffuse = saturate(dot(input.norm, -light));
	float ambient = 0.2f;
	float3 color = float3(1.0f, 1.0f, 1.0f);

	return float4(saturate((color * diffuse) + (color * ambient)), 1.0f);
}

VS_OUTPUT2 VS2(float3 input : POSITION)
{
	VS_OUTPUT2 output;

	float scale = height / 1000;
	float4 mysample = heightmap.Load(int3(input));
	output.pos = float4(input.x, input.y, mysample.r * scale, 1.0f);
	output.tex = float4(input.x / height, input.y / width, output.pos.z, scale);
	output.pos = mul(output.pos, viewproj);

	float zb = heightmap.Load(int3(input.xy + int2(0, -1), 0)).r * scale;
	float zc = heightmap.Load(int3(input.xy + int2(1, 0), 0)).r * scale;
	float zd = heightmap.Load(int3(input.xy + int2(1, 1), 0)).r * scale;
	float ze = heightmap.Load(int3(input.xy + int2(0, 1), 0)).r * scale;
	float zf = heightmap.Load(int3(input.xy + int2(-1, 0), 0)).r * scale;
	float zg = heightmap.Load(int3(input.xy + int2(-1, -1), 0)).r * scale;

	float x = 2 * zf + zc + zg - zb - 2 * zc - zd;
	float y = 2 * zb + zc + zg - zd - 2 * ze - zf;
	float z = 6.0f;

	output.norm = float4(normalize(float3(x, y, z)), 1.0f);

	output.worldpos = float4(input, 1.0f);

	return output;
}

