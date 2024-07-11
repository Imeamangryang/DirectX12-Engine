#pragma once
#include "stdafx.h"
#include "Light.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace std;

struct ConstantBuffer
{
	XMFLOAT4X4 world;
	XMFLOAT4X4 viewproj;
	XMFLOAT4 eye;
	LightSource light;
	UINT height;
	UINT width;
};

struct SkyConstantBuffer
{
	XMFLOAT4X4 world;
	XMFLOAT4X4 viewproj;
	XMFLOAT4 eye;
	UINT height;
	UINT width;
};

struct Vertex
{
	Vertex() {}
	Vertex(
		float px, float py, float pz,
		float nx, float ny, float nz,
		float tx, float ty, float tz,
		float u, float v) :
		Position(px, py, pz),
		Normal(nx, ny, nz),
		TangentU(tx, ty, tz),
		TexC(u, v) {}

	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	XMFLOAT3 TangentU;
	XMFLOAT2 TexC;
	XMFLOAT4 BoneWeights;
	XMFLOAT4 BoneIndices;
};

struct FbxMaterialInfo
{
	XMFLOAT4		diffuse;
	XMFLOAT4		ambient;
	XMFLOAT4		specular;
	wstring			name;
	wstring			diffuseTexName;
	wstring			normalTexName;
	wstring			specularTexName;
};

struct BoneWeight
{
	using Pair = pair<UINT, double>;
	vector<Pair> boneWeights;

	void AddWeights(UINT index, double weight)
	{
		if (weight <= 0.f)
			return;

		auto findIt = std::find_if(boneWeights.begin(), boneWeights.end(),
			[=](const Pair& p) { return p.second < weight; });

		if (findIt != boneWeights.end())
			boneWeights.insert(findIt, Pair(index, weight));
		else
			boneWeights.push_back(Pair(index, weight));

		// ����ġ�� �ִ� 4��
		if (boneWeights.size() > 4)
			boneWeights.pop_back();
	}

	void Normalize()
	{
		double sum = 0.f;
		for_each(boneWeights.begin(), boneWeights.end(), [&](Pair& p) { sum += p.second; });
		for_each(boneWeights.begin(), boneWeights.end(), [=](Pair& p) { p.second = p.second / sum; });
	}
};

struct FbxMeshInfo
{
	wstring								name;
	vector<Vertex>						vertices;
	vector<vector<UINT>>				indices;
	vector<FbxMaterialInfo>				materials;
	vector<BoneWeight>					boneWeights; // �� ����ġ
	bool								hasAnimation;
};

struct FbxKeyFrameInfo
{
	FbxAMatrix  matTransform;
	double		time;
};

struct FbxBoneInfo
{
	wstring					boneName;
	UINT					parentIndex;
	FbxAMatrix				matOffset;
};

struct FbxAnimClipInfo
{
	wstring			name;
	FbxTime			startTime;
	FbxTime			endTime;
	FbxTime::EMode	mode;
	vector<vector<FbxKeyFrameInfo>>	keyFrames;
};

struct FbxMeshData {
	std::string MeshName = "";
	UINT VertexSize = 0;
	UINT IndexSize = 0;
};