struct VS_OUTPUT
{
	float3 pos : POSITION;
	float3 norm : NORMAL;
	float3 tan : TANGENT;
};

// Output control point
struct HS_CONTROL_POINT_OUTPUT
{
	float3 pos : POSITION;
	float3 norm : NORMAL;
	float3 tan : TANGENT;
};

// Output patch constant data.
struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[3]	: SV_TessFactor; 
	float InsideTessFactor : SV_InsideTessFactor;
};

#define NUM_CONTROL_POINTS 3

HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<VS_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT output;

	// 테셀레이션 수준 결정
	output.EdgeTessFactor[0] = 3;
	output.EdgeTessFactor[1] = 3;
	output.EdgeTessFactor[2] = 3;
	output.InsideTessFactor = 3;

	return output;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcHSPatchConstants")]
HS_CONTROL_POINT_OUTPUT HS(
	InputPatch<VS_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID)
{
	HS_CONTROL_POINT_OUTPUT output;

	// Insert code to compute Output here
	output.pos = ip[i].pos;
	output.norm = ip[i].norm;
	output.tan = ip[i].tan;

	return output;
}
