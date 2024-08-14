#define NUM_CONTROL_POINTS 3
Texture2D Tex_table[300] : register(t0);

SamplerState dmsampler : register(s0);
SamplerState cmsampler : register(s1);

struct VS_INPUT
{
    float3 pos : POSITION;
    float3 norm : NORMAL;
    float3 tan : TANGENT;
    float2 tex : TEXCOORD;
    uint instanceID : SV_InstanceID;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float3 norm : NORMAL;
    float3 tan : TANGENT;
    float2 tex : TEXCOORD;
    uint instanceID : SV_InstanceID;
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
    bool ispicking;
}

struct InstanceBuffer
{
    float4x4 instanceTrans;
    int blocktype;
    bool isvisible;
};

StructuredBuffer<InstanceBuffer> instanceTransforms : register(t4); // 인스턴스 변환 행렬

// blocktype에 따라 다른 색상을 가지도록 설정
float4 GetColor(int blocktype, float2 tex)
{
    if (blocktype == 0) return float4(1.0f, 0.0f, 0.0f, 1.0f);
    else if (blocktype == 1) return float4(Tex_table[0].Sample(cmsampler, tex));
    else if (blocktype == 2) return float4(Tex_table[1].Sample(cmsampler, tex));
    else if (blocktype == 3) return float4(Tex_table[2].Sample(cmsampler, tex));
    else if (blocktype == 5) return float4(Tex_table[3].Sample(cmsampler, tex));
    else return float4(0.0f, 0.0f, 0.0f, 1.0f);
}

// Vertex shader
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;
    
    output.pos = float4(input.pos, 1.0f);
    output.pos += mul(instanceTransforms[input.instanceID].instanceTrans, output.pos);
 
    output.pos = mul(world, output.pos);
    output.pos = mul(output.pos, viewproj);
    
    output.norm = input.norm;
    
    output.tan = input.tan;
    
    output.tex = input.tex;
    
    output.instanceID = input.instanceID;

    return output;
}

// Pixel shader
float4 PS(VS_OUTPUT input) : SV_TARGET
{
    if(instanceTransforms[input.instanceID].isvisible == 1)
    {
        return float4(1.0f, 0.0f, 0.0f, 1.0f);
    }
    float3 norm = input.norm;
    norm = normalize(norm);
    
    float4 color = GetColor(instanceTransforms[input.instanceID].blocktype, input.tex);
    
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
