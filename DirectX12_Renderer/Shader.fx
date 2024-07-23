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
    float3 pos : POSITION;
    float3 norm : NORMAL;
    float3 tan : TANGENT;
    float2 tex : TEXCOORD;
};

struct HS_CONTROL_POINT_OUTPUT
{
    float3 pos : POSITION;
    float3 norm : NORMAL;
    float3 tan : TANGENT;
    float2 tex : TEXCOORD;
};

struct HS_CONSTANT_DATA_OUTPUT
{
    float EdgeTessFactor[3] : SV_TessFactor;
    float InsideTessFactor : SV_InsideTessFactor;
    
    // Geometry cubic generated control points
    float3 f3B210 : POSITION3;
    float3 f3B120 : POSITION4;
    
    float3 f3B021 : POSITION5;
    float3 f3B012 : POSITION6;
    
    float3 f3B102 : POSITION7;
    float3 f3B201 : POSITION8;
    
    float3 f3B111 : CENTER;
    
    // Normal quadratic generated control points
    float3 f3N110 : NORMAL3;
    float3 f3N011 : NORMAL4;
    float3 f3N101 : NORMAL5;
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
    int edgeTessellationFactor1;
    int edgeTessellationFactor2;
    int edgeTessellationFactor3;
    int insideTessellationFactor;
}

// Vertex shader
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;
    
    output.pos = input.pos;
    
    output.norm = input.norm;
    //output.norm = normalmap.SampleLevel(dmsampler, input.tex, 0).xyz;
    
    output.tan = input.tan;
    
    output.tex = input.tex;

    return output;
}

// Hull shader
HS_CONSTANT_DATA_OUTPUT HS_Constant(
	InputPatch<VS_OUTPUT, NUM_CONTROL_POINTS> ip)
{
    HS_CONSTANT_DATA_OUTPUT Output;
    
    // PN 또는 PN-AEN 트라이앵글의 제어점 단계에서 계산
    float3 f3B003 = ip[0].pos;
    float3 f3B030 = ip[1].pos;
    float3 f3B300 = ip[2].pos;
    
    // And Normals
    float3 f3N002 = ip[0].norm;
    float3 f3N020 = ip[1].norm;
    float3 f3N200 = ip[2].norm;
    
    Output.EdgeTessFactor[0] = edgeTessellationFactor1;
    Output.EdgeTessFactor[1] = edgeTessellationFactor2;
    Output.EdgeTessFactor[2] = edgeTessellationFactor3;
    Output.InsideTessFactor = insideTessellationFactor;
    
    Output.f3B210 = ((2.0f * f3B003) + f3B030 - (dot((f3B030 - f3B003), f3N002) * f3N002)) / 3.0f;
    Output.f3B120 = ((2.0f * f3B030) + f3B003 - (dot((f3B003 - f3B030), f3N020) * f3N020)) / 3.0f;
    Output.f3B021 = ((2.0f * f3B030) + f3B300 - (dot((f3B300 - f3B030), f3N020) * f3N020)) / 3.0f;
    Output.f3B012 = ((2.0f * f3B300) + f3B030 - (dot((f3B030 - f3B300), f3N200) * f3N200)) / 3.0f;
    Output.f3B102 = ((2.0f * f3B300) + f3B003 - (dot((f3B003 - f3B300), f3N200) * f3N200)) / 3.0f;
    Output.f3B201 = ((2.0f * f3B003) + f3B300 - (dot((f3B300 - f3B003), f3N002) * f3N002)) / 3.0f;

    // Center Control Point
    float3 f3E = (Output.f3B210 + Output.f3B120 + Output.f3B021 + Output.f3B012 + Output.f3B102 + Output.f3B201) / 6.0f; // 추가된 제어점의 평균
    float3 f3V = (f3B003 + f3B030 + f3B300) / 3.0f; // 기존의 제어점의 평균
    
    Output.f3B111 = f3E + ((f3E - f3V) / 2.0f);
    
    float fV12 = 2.0f * dot(f3B030 - f3B003, f3N002 + f3N020) / dot(f3B030 - f3B003, f3B030 - f3B003);
    Output.f3N110 = normalize(f3N002 + f3N020 - fV12 * (f3B030 - f3B003));
    float fV23 = 2.0f * dot(f3B300 - f3B030, f3N020 + f3N200) / dot(f3B300 - f3B030, f3B300 - f3B030);
    Output.f3N011 = normalize(f3N020 + f3N200 - fV23 * (f3B300 - f3B030));
    float fV31 = 2.0f * dot(f3B003 - f3B300, f3N200 + f3N002) / dot(f3B003 - f3B300, f3B003 - f3B300);
    Output.f3N101 = normalize(f3N200 + f3N002 - fV31 * (f3B003 - f3B300));
    
    return Output;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("HS_Constant")]
HS_CONTROL_POINT_OUTPUT HS(
	InputPatch<VS_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint i : SV_OutputControlPointID)
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
    
    // The barycentric coordinates
    float fU = domain.x;
    float fV = domain.y;
    float fW = domain.z;
    
    // Precompute squares and squares * 3
    float fUU = fU * fU;
    float fVV = fV * fV;
    float fWW = fW * fW;
    float fUU3 = fUU * 3.0f;
    float fVV3 = fVV * 3.0f;
    float fWW3 = fWW * 3.0f;
    
    float3 f3EyePosition = patch[0].pos * fWW * fW +
                           patch[1].pos * fUU * fU +
                           patch[2].pos * fVV * fV +
                           
                           input.f3B210 * fWW3 * fU +
                           input.f3B120 * fW * fUU3 +
                           input.f3B201 * fWW3 * fV +
                           input.f3B021 * fUU3 * fV +
                           input.f3B102 * fW * fVV3 +
                           input.f3B012 * fU * fVV3 +
    
                           input.f3B111 * 6.0f * fW * fU * fV;
    
    Output.pos = float4(f3EyePosition, 1.0f);
    
    // Compute normal from quadratic control points and barycentric coords
    float3 f3Normal = patch[0].norm * fWW +
                      patch[1].norm * fUU +
                      patch[2].norm * fVV +
                      input.f3N110 * fW * fU +
                      input.f3N011 * fU * fV +
                      input.f3N101 * fW * fV;
   
    Output.norm = normalize(f3Normal);
    
    Output.tan = normalize(patch[0].tan.xyz * domain.x + patch[1].tan.xyz * domain.y + patch[2].tan.xyz * domain.z);

    Output.tex = float2(patch[0].tex.xy * domain.x + patch[1].tex.xy * domain.y + patch[2].tex.xy * domain.z);
    
    float3 norm = normalmap.SampleLevel(dmsampler, Output.tex, 0).xyz;
    
    Output.pos.xyz += norm * 0.3f;
   
    Output.pos = mul(world, Output.pos);    // Transform to world space
    Output.pos = mul(Output.pos, viewproj); // Transform to homogeneous clip space
    
    Output.norm = normalize(mul(world, float4(Output.norm, 0.0f)).xyz);
    Output.norm = normalize(mul(float4(Output.norm, 0.0f), viewproj).xyz);
    
    return Output;
}

// Pixel shader
float4 PS(DS_OUTPUT input) : SV_TARGET
{
    float3 norm = normalmap.Sample(cmsampler, input.tex).xyz + input.norm.rgb;
    norm = normalize(norm);
    
    float4 color = float4(colormap.Sample(cmsampler, input.tex));
    
    float4 ambient = color * light.amb;
    float4 diffuse = color * light.dif * dot(-light.dir, norm);
    float3 V = reflect(light.dir, norm);
    float3 toEye = normalize(eye.xyz - input.pos.xyz);
    float4 specular = color * 0.1f * light.spec * pow(max(dot(V, toEye), 0.0f), 1.0f);
    
    //return saturate(ambient + diffuse + specular);
    return saturate(float4(norm, 1.0f));
}
