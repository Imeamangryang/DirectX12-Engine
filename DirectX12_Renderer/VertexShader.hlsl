Texture2D<float4> heightmap : register(t0);

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 norm : NORMAL;
    float3 tan : TANGENT;
    float2 tex : TEXCOORD;
};

struct VS_INPUT
{
    float3 pos : POSITION;
    float3 norm : NORMAL;
    float3 tan : TANGENT;
    float2 tex : TEXCOORD;
};

cbuffer ConstantBuffer : register(b0)
{
    float4x4 world;
    float4x4 viewproj;
    float4 eye;
    int height;
    int width;
}

VS_OUTPUT VS(VS_INPUT input) {
    VS_OUTPUT output;

    // 월드 변환 적용
    float4 worldPos = mul(world, float4(input.pos, 1.0f));
    // 뷰-프로젝션 변환 적용
    output.pos = mul(worldPos, viewproj);

    // 노멀 변환 (월드 행렬의 상위 3x3 부분만 사용)
    float3 worldNormal = mul(input.norm, (float3x3) world);
    output.norm = float4(worldNormal, 1.0f);
    
    output.tan = input.tan;
    
    output.tex = input.tex;

    return output;
}
