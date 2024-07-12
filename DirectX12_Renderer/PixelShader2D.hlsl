Texture2D<float4> colormap : register(t0);
SamplerState hmsampler : register(s0);
SamplerState cmsampler : register(s1);


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

float4 PS2D(VS_OUTPUT input) : SV_TARGET{
	
    return colormap.Sample(cmsampler, input.tex);

}