#define NUM_CONTROL_POINTS 3

Texture2D<float4> colormap : register(t0);
Texture2D<float4> normalmap : register(t1);
SamplerState Linearsampler : register(s0);
SamplerState Pointsampler : register(s1);

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
}

cbuffer AnimationConstantBuffer : register(b1)
{
    matrix boneTransforms[200];
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

// Pixel shader
float4 PS(VS_OUTPUT input) : SV_TARGET
{
    // 원래 색상 계산
    float3 norm = input.norm;
    norm = normalize(norm);
    
    float4 color = colormap.Sample(Linearsampler, input.tex);

    // World space에서의 light pos와 dir 계산
    float3 worldlightpos = mul(light.pos, world).xyz;
    float3 lightDir = normalize(input.pos.xyz - worldlightpos);

    float4 ambient = light.amb;
    float4 diffuse = light.dif * dot(-lightDir, norm);
    float3 V = reflect(lightDir, norm);
    float3 toEye = normalize(eye.xyz - input.pos.xyz);
    float4 specular = 0.1f * light.spec * pow(max(dot(V, toEye), 0.0f), 4.0f);

    float4 finalColor = float4(saturate((ambient + diffuse + specular).rgb * color.rgb), color.a);
    
    return finalColor;
}
