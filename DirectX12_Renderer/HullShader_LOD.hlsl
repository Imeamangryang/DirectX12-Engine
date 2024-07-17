cbuffer CharacterConstantBuffer : register(b1)
{
    int edgeTessellationFactor;
    int insideTessellationFactor;
}

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float3 norm : NORMAL;
    float3 tan : TANGENT;
    float2 tex : TEXCOORD;
};

struct HS_CONTROL_POINT_OUTPUT
{
    float4 pos : SV_POSITION;
    float3 norm : NORMAL;
    float3 tan : TANGENT;
    float2 tex : TEXCOORD;
};

struct HS_CONSTANT_DATA_OUTPUT
{
    float EdgeTessFactor[3] : SV_TessFactor;
    float InsideTessFactor : SV_InsideTessFactor;
};

#define NUM_CONTROL_POINTS 3

HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<VS_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
    HS_CONSTANT_DATA_OUTPUT Output;

    Output.EdgeTessFactor[0] = edgeTessellationFactor;
    Output.EdgeTessFactor[1] = edgeTessellationFactor;
    Output.EdgeTessFactor[2] = edgeTessellationFactor;
    Output.InsideTessFactor = insideTessellationFactor;

    return Output;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcHSPatchConstants")]
HS_CONTROL_POINT_OUTPUT HS_LOD(
	InputPatch<VS_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID)
{
    HS_CONTROL_POINT_OUTPUT Output;

    Output.pos = ip[i].pos;
    Output.norm = ip[i].norm;
    Output.tan = ip[i].tan;
    Output.tex = ip[i].tex;

    return Output;
}
