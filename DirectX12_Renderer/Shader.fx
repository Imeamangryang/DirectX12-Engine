#define NUM_CONTROL_POINTS 3

Texture2D<float4> colormap : register(t0);
Texture2D<float4> normalmap : register(t1);
Texture2D<float4> heightmap : register(t2);
SamplerState dmsampler : register(s0);
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

struct HS_CONTROL_POINT_OUTPUT
{
    float4 pos : SV_POSITION;
    float3 norm : NORMAL;
    float3 tan : TANGENT;
    float2 tex : TEXCOORD;
};

struct HS_CONSTANT_DATA_OUTPUT
{
    float EdgeTessFactor[3] : SV_TessFactor;
    float InsideTessFactor : SV_InsideTessFactor;
};

struct DS_OUTPUT
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

// Vertex shader
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;
    
    output.pos = float4(input.pos, 1.0f);
    
    output.norm = input.norm;
    
    output.tan = input.tan;
    
    output.tex = input.tex;

    return output;
}

// Hull shader
HS_CONSTANT_DATA_OUTPUT HS_Constant(
	InputPatch<VS_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
    HS_CONSTANT_DATA_OUTPUT Output;

    Output.EdgeTessFactor[0] = 9;
    Output.EdgeTessFactor[1] = 9;
    Output.EdgeTessFactor[2] = 9;
    Output.InsideTessFactor = 9;

    return Output;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("HS_Constant")]
HS_CONTROL_POINT_OUTPUT HS(
	InputPatch<VS_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID)
{
    HS_CONTROL_POINT_OUTPUT Output;

    Output.pos = ip[i].pos;
    Output.norm = ip[i].norm;
    Output.tan = ip[i].tan;
    Output.tex = ip[i].tex;

    return Output;
}


// Domain shader
[domain("tri")]
DS_OUTPUT DS(
    HS_CONSTANT_DATA_OUTPUT input,
    float3 domain : SV_DomainLocation,
    const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> patch)
{
    DS_OUTPUT Output;

    Output.pos = float4(patch[0].pos.xyz * domain.x + patch[1].pos.xyz * domain.y + patch[2].pos.xyz * domain.z, 1);
    
    Output.norm = normalize(patch[0].norm.xyz * domain.x + patch[1].norm.xyz * domain.y + patch[2].norm.xyz * domain.z);
    
    Output.tan = normalize(patch[0].tan.xyz * domain.x + patch[1].tan.xyz * domain.y + patch[2].tan.xyz * domain.z);

    Output.tex = float2(patch[0].tex.xy * domain.x + patch[1].tex.xy * domain.y + patch[2].tex.xy * domain.z);
    
    //Output.norm += normalmap.SampleLevel(dmsampler, Output.tex, 0).xyz;
    //float height = heightmap.SampleLevel(dmsampler, Output.tex.xy, 0).r;
    
    //Output.pos.xyz += Output.norm * height;
    
    Output.pos = mul(world, Output.pos);
    Output.pos = mul(Output.pos, viewproj);
    
    Output.norm = mul((float3x3)world, Output.norm);
    
    return Output;
}

// Pixel shader
float4 PS(DS_OUTPUT input) : SV_TARGET
{
    
    float3 norm = normalmap.Sample(cmsampler, input.tex).xyz + input.norm;
    norm = normalize(norm);
    
    float4 color = float4(colormap.Sample(cmsampler, input.tex));
    
    // World space에서의 light pos와 dir 계산
    float3 worldlightpos = mul(light.pos, world).xyz;
    float3 lightDir = normalize(input.pos.xyz - worldlightpos);
    
    float4 ambient = light.amb;
    float4 diffuse = light.dif * dot(-lightDir, norm);
    float3 V = reflect(lightDir, norm);
    float3 toEye = normalize(eye.xyz - input.pos.xyz);
    float4 specular = 0.1f * light.spec * pow(max(dot(V, toEye), 0.0f), 4.0f);
    
    return float4(saturate((ambient + diffuse + specular).rgb * color.rgb), color.a);
    //return saturate(normalmap.SampleLevel(cmsampler, input.tex, 0));
}
