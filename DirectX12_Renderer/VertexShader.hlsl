Texture2D<float4> heightmap : register(t0);

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float4 norm : NORMAL;
	float2 tex : TEXCOORD;
};

struct VS_INPUT
{
	float3 pos : POSITION;
	float3 norm : NORMAL;
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

	// ���� ��ȯ ����
    float4 worldPos = mul(world, float4(input.pos, 1.0f));
    // ��-�������� ��ȯ ����
    output.pos = mul(worldPos, viewproj);

	// ��� ��ȯ (���� ����� ���� 3x3 �κи� ���)
    float3 worldNormal = mul(input.norm, (float3x3) world);
    output.norm = float4(worldNormal, 1.0f);

    // �ؽ�ó ��ǥ ���
    float theta = atan2(worldNormal.x, worldNormal.z);
    float phi = acos(worldNormal.y);

	output.tex.x = theta / (2.0f * 3.14f);
	output.tex.y = phi / 3.14f;

	return output;
}
