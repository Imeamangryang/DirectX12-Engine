Texture2D<float4> normalmap : register(t1);
SamplerState dmsampler : register(s0);

cbuffer ConstantBuffer : register(b0)
{
    float4x4 world;
    float4x4 viewproj;
    float4 eye;
    int height;
    int width;
}

struct DS_OUTPUT
{
    float4 pos : SV_POSITION;
    float3 norm : NORMAL;
    float3 tan : TANGENT;
    float2 tex : TEXCOORD;
};

struct HS_CONTROL_POINT_OUTPUT
{
    float4 pos : SV_POSITION;
    float3 norm : NORMAL;
    float3 tan : TANGENT;
    float2 tex : TEXCOORD;
};

struct HS_CONSTANT_DATA_OUTPUT
{
    float EdgeTessFactor[3]			: SV_TessFactor;
    float InsideTessFactor			: SV_InsideTessFactor;
};

#define NUM_CONTROL_POINTS 3

[domain("tri")]
DS_OUTPUT DS_DRAGON(
    HS_CONSTANT_DATA_OUTPUT input,
    float3 domain : SV_DomainLocation,
    const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> patch)
{
    DS_OUTPUT Output;

    Output.pos = float4(patch[0].pos.xyz * domain.x + patch[1].pos.xyz * domain.y + patch[2].pos.xyz * domain.z, 1);
    
    Output.norm = normalize(patch[0].norm.xyz * domain.x + patch[1].norm.xyz * domain.y + patch[2].norm.xyz * domain.z);
    
    Output.tan = normalize(patch[0].tan.xyz * domain.x + patch[1].tan.xyz * domain.y + patch[2].tan.xyz * domain.z);

    Output.tex = float2(patch[0].tex.xy * domain.x + patch[1].tex.xy * domain.y + patch[2].tex.xy * domain.z);
    
    //// Output.norm의 크기로부터 Height Scale을 구한다.
    //float heightScale = length(Output.norm);
    
    //// Height Scale을 이용하여 높이를 계산한다.
    //float3 normal = normalmap.SampleLevel(dmsampler, Output.tex, 0).xyz;
    
    //// 높이를 적용한다.
    //Output.pos.xyz += normal * heightScale;
    
    //Output.pos = float4(Output.pos.xyz, 1.0f);
    
    Output.pos = mul(world, Output.pos);
    Output.pos = mul(Output.pos, viewproj);
    
    return Output;
}
