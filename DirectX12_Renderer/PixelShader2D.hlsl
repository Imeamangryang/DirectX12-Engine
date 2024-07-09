Texture2D<float4> heightmap : register(t0);
Texture2D<float4> colormap : register(t1);
SamplerState hmsampler : register(s0);
SamplerState cmsampler : register(s1);


struct VS_OUTPUT {
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD;
};

float4 PS2D(VS_OUTPUT input) : SV_TARGET{
	//float height = heightmap.Sample(hmsampler, input.tex).r;
	//return float4(height, height, height, 1);
	//float color = colormap.Sample(cmsampler, input.tex).r;
	return colormap.Sample(cmsampler, input.tex);

}