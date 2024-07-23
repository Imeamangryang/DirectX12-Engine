#pragma once
#include "stdafx.h"
#include "Structures.h"
#include <fstream>
#include <iomanip>

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace std;

class FBXLoader
{
public:
	FBXLoader();
	~FBXLoader();

public:
	void LoadFbx(const string& path);

public:
	UINT GetMeshCount() { return static_cast<UINT>(_meshes.size()); }
	const FbxMeshInfo& GetMesh(UINT idx) { return _meshes[idx]; }
	vector<shared_ptr<FbxBoneInfo>>& GetBones() { return _bones; }
	vector<shared_ptr<FbxAnimClipInfo>>& GetAnimClip() { return _animClips; }
private:
	void Import(const string path);

	void ParseNode(FbxNode* root);
	void LoadMesh(FbxMesh* mesh);
	void LoadMaterial(FbxSurfaceMaterial* surfaceMaterial);

	void		GetNormal(FbxMesh* mesh, FbxMeshInfo* container, UINT idx, UINT vertexCounter);
	void		GetTangent(FbxMesh* mesh, FbxMeshInfo* container, UINT idx, UINT vertexCounter);
	void		GetUV(FbxMesh* mesh, FbxMeshInfo* container, UINT idx, UINT vertexCounter);
	XMFLOAT4	GetMaterialData(FbxSurfaceMaterial* surface, const char* materialName, const char* factorName);
	string		GetTextureRelativeName(FbxSurfaceMaterial* surface, const char* materialProperty);

	// Animation
	void LoadBones(FbxNode* node) { LoadBones(node, 0, -1); }
	void LoadBones(FbxNode* node, UINT idx, UINT parentIdx);
	void LoadAnimationInfo();

	void LoadAnimationData(FbxMesh* mesh, FbxMeshInfo* meshInfo);
	void LoadBoneWeight(FbxCluster* cluster, UINT boneIdx, FbxMeshInfo* meshInfo);
	void LoadOffsetMatrix(FbxCluster* cluster, const FbxAMatrix& matNodeTransform, UINT boneIdx, FbxMeshInfo* meshInfo);
	void LoadKeyframe(UINT animIndex, FbxNode* node, FbxCluster* cluster, const FbxAMatrix& matNodeTransform, UINT boneIdx, FbxMeshInfo* container);

	UINT FindBoneIndex(string name);
	FbxAMatrix GetTransform(FbxNode* node);

	void FillBoneWeight(FbxMesh* mesh, FbxMeshInfo* meshInfo);

	// File IO
	void SaveMeshInfoToFile(const std::string& filename);
	void SaveBonesToFile(const std::string& filename);
	void SaveAnimationClipsToFile(const std::string& filename);

private:
	FbxManager* _manager = nullptr;
	FbxScene* _scene = nullptr;
	FbxImporter* _importer = nullptr;
	wstring			_resourceDirectory;

	vector<FbxMeshInfo>					_meshes;
	vector<shared_ptr<FbxBoneInfo>>		_bones;
	vector<shared_ptr<FbxAnimClipInfo>>	_animClips;
	FbxArray<FbxString*>				_animNames;
};
