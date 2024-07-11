struct VS_INPUT
{
	float3 pos : POSITION;
};

struct VS_OUTPUT {
	float4 pos : SV_POSITION;
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

	// 월드 변환 적용
    float4 worldPos = mul(world, float4(input.pos, 1.0f));
    // 뷰-프로젝션 변환 적용
    output.pos = mul(worldPos, viewproj);

	return output;
}