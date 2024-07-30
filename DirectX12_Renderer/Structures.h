#pragma once
#include "stdafx.h"
#include "Light.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace std;

enum class BlockType {
	Air,
	Dirt,
	Stone,
	Cobblestone,
	Oak_planks
};

struct ConstantBuffer
{
	XMFLOAT4X4 world;
	XMFLOAT4X4 viewproj;
	XMFLOAT4 eye;
	LightSource light;
	UINT blockType;
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
	XMFLOAT4 BoneIndices;
	XMFLOAT4 BoneWeights;
};

struct FbxMaterialInfo
{
	XMFLOAT4		diffuse;
	XMFLOAT4		ambient;
	XMFLOAT4		specular;
	string			name;
	string			diffuseTexName;
	string			normalTexName;
	string			specularTexName;
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

		// 가중치는 최대 4개
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
	std::string							name;
	vector<Vertex>						vertices;
	vector<std::uint32_t>				indices;
	vector<FbxMaterialInfo>				materials;
	vector<BoneWeight>					boneWeights; // 뼈 가중치
	bool								hasAnimation;
};

struct FbxKeyFrameInfo
{
	FbxAMatrix  matTransform;
	double		time;
};

struct FbxBoneInfo
{
	string					boneName;
	UINT					parentIndex;
	UINT					childcount;
	FbxAMatrix				matOffset;
};

struct FbxAnimClipInfo
{
	string			name;
	FbxTime			startTime;
	FbxTime			endTime;
	FbxTime::EMode	mode;
	vector<vector<FbxKeyFrameInfo>>	keyFrames;
};

struct FbxMeshData {
	std::string MeshName = "";
	UINT VertexSize = 0;
	UINT IndexSize = 0;
	UINT StartIndexLocation = 0;
	UINT BaseVertexLocation = 0;
};

struct KeyFrameInfo
{
	double	time;
	UINT	frame;
	XMFLOAT3	scale;
	XMFLOAT4	rotation;
	XMFLOAT3	translate;
	XMFLOAT4X4	matTransform;
};

struct BoneInfo
{
	string					boneName;
	UINT					parentIdx;
	XMFLOAT4X4				matOffset;
};

struct AnimClipInfo
{
	string			animName;
	UINT			frameCount;
	double			duration;
	vector<vector<KeyFrameInfo>>	keyFrames;
};

struct CharacterConstantBuffer
{
	int edgeTessellationFactor1 = 1;
	int edgeTessellationFactor2 = 1;
	int edgeTessellationFactor3 = 1;
	int insideTessellationFactor = 1;

	XMFLOAT4X4 BoneTransforms[96];
};