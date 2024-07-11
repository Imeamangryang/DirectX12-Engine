Texture2D<float4> heightmap : register(t0);
Texture2D<float4> colormap : register(t1);
SamplerState hmsampler : register(s0);
SamplerState cmsampler : register(s1);


struct VS_OUTPUT {
	float4 pos : SV_POSITION;
};

float4 PS2D(VS_OUTPUT input) : SV_TARGET{
	
    return float4(1.0f, 1.0f, 1.0f, 1.0f);

}