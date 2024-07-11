struct VS_INPUT
{
    float3 pos : POSITION;
    float3 norm : NORMAL;
    float3 tan : TANGENT;
    float2 tex : TEXCOORD;
};

struct VS_OUTPUT {
	float4 pos : SV_POSITION;
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

VS_OUTPUT VS2D(VS_INPUT input) {
	VS_OUTPUT output;

	// ���� ��ȯ ����
    float4 worldPos = mul(world, float4(input.pos, 1.0f));
    // ��-�������� ��ȯ ����
    output.pos = mul(worldPos, viewproj);
    
    output.tex = input.tex;

	return output;
}