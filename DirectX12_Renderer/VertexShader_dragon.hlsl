Texture2D<float4> colormap : register(t0);
Texture2D<float4> normalmap : register(t1);
SamplerState hmsampler : register(s0);
SamplerState cmsampler : register(s1);

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

struct LightData
{
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

VS_OUTPUT VS_DRAGON(VS_INPUT input)
{
    VS_OUTPUT output;

	//// 월드 변환 적용
    //float4 worldPos = mul(world, float4(input.pos, 1.0f));
    //// 뷰-프로젝션 변환 적용
    //output.pos = mul(worldPos, viewproj);
    
    output.pos = float4(input.pos, 1.0f);
    
    output.norm = input.norm;
    //output.norm = normalmap.SampleLevel(hmsampler, input.tex, 0.0f).xyz;
    
    output.tan = input.tan;
    
    output.tex = input.tex;

    return output;
}