Texture2D<float4> displacementmap : register(t0);
Texture2D<float4> colormap : register(t1);
SamplerState dmsampler : register(s0);
SamplerState cmsampler : register(s1);


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

float4 PSTes(DS_OUTPUT input) : SV_TARGET
{
	float3 norm = input.norm.xyz;
	
    float theta = atan2(input.norm.z, input.norm.x);
    if (theta < 0.0f)
        theta += 2.0f * 3.14159265359f;
    float phi = acos(input.norm.y);
    float2 tex = float2(theta / (2.0f * 3.14159265359f), phi / 3.14159265359f);
	
	float4 color = float4(colormap.Sample(cmsampler, tex));

	float4 ambient = color * light.amb;
	float4 diffuse = color * light.dif * dot(-light.dir, norm);
	float3 V = reflect(light.dir, norm);
	float3 toEye = normalize(eye.xyz - input.pos.xyz);
	float4 specular = color * 0.1f * light.spec * pow(max(dot(V, toEye), 0.0f), 1.0f);

    return saturate(ambient + diffuse + specular);
}