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
    //float3 tan : TANGENT;
    float2 tex : TEXCOORD;
};

struct HS_CONTROL_POINT_OUTPUT
{
    float4 pos : SV_POSITION;
    //float3 tan : TANGENT;
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
    Output.pos = mul(world, Output.pos);
    Output.pos = mul(Output.pos, viewproj);

    Output.tex = float2(patch[0].tex.xy * domain.x + patch[1].tex.xy * domain.y + patch[2].tex.xy * domain.z);

    
    // H(x, y) = {H(x+1,y) + H(x-1, y) + H(x, y+1) + H(x, y-1)}/4 + {N_x(x+1, y) - N_x(x-1, y) + N_y(x, y+1) - N_y(x, y-1)}/8

    
    return Output;
}
