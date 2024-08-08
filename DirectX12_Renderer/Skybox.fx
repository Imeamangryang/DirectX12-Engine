Texture2D<float4> skymap : register(t0);
SamplerState skySampler : register(s0);

struct VS_INPUT
{
    float3 pos : POSITION;
    float3 norm : NORMAL;
    float3 tan : TANGENT;
    float2 tex : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float3 norm : NORMAL;
    float3 tan : TANGENT;
    float2 tex : TEXCOORD;
};

cbuffer ConstantBuffer : register(b0)
{
    float4x4 world;
    float4x4 viewproj;
}

// Vertex shader
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;
    
    output.pos = float4(input.pos, 1.0f);
 
    output.pos = mul(world, output.pos);
    output.pos = mul(output.pos, viewproj);
    
    output.norm = input.norm;
    
    output.tan = input.tan;
    
    output.tex = input.tex;
    
    return output;
}

//Pixel shader
float4 PS(VS_OUTPUT input) : SV_TARGET
{
    // 텍스처 좌표 계산
    float theta = atan2(input.norm.x, input.norm.y);
    float phi = acos(input.norm.z);
	
    float2 tex = float2(theta / 6.28318530718f, phi / 3.14159265359f);
    
    return skymap.Sample(skySampler, tex);
}