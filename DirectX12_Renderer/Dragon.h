#pragma once
#include "Renderer.h"
#include "stdafx.h"
#include "Structures.h"
#include "Object.h"
#include "FBXLoader.h"
#include "DirectionalLight.h"
#include "Texture.h"
#include <fstream>
#include <iomanip>

using namespace graphics;

struct AnimFrameParams
{
	XMFLOAT3 scale;
	XMFLOAT4 rotation;
	XMFLOAT3 translation;
};

struct AnimationConstantBuffer
{
	XMMATRIX BoneTransforms[200];
};

class Dragon : public Object
{
public:
	Dragon(Graphics* renderer);
	~Dragon();

	void Draw(ComPtr<ID3D12GraphicsCommandList> m_commandList, XMFLOAT4X4 viewproj, XMFLOAT4 eye);

	void SetIsWireframe(bool isWireframe) { this->isWireframe = isWireframe; }


	vector<shared_ptr<FbxBoneInfo>> m_boneInfos;

private:
	void InitPipeline(Graphics* Renderer);

	void CreateDescriptorHeap(Graphics* Renderer);

	void LoadFBXModel(Graphics* Renderer, string path);

	XMFLOAT4X4 GetMatrix(FbxAMatrix& mat);

	XMMATRIX Interpolate(const KeyFrameInfo& kf1, const KeyFrameInfo& kf2, double time);
	void ComputeBoneTransforms(const AnimClipInfo& animClip, double time, const vector<BoneInfo>& bones, vector<XMMATRIX>& boneTransforms);


	ComPtr<ID3D12DescriptorHeap> m_srvHeap;

	UINT m_width;
	UINT m_height;

	bool isWireframe = false;
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12PipelineState> m_pipelineStateWireframe;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12Resource> m_CBV;
	ConstantBuffer m_constantBufferData;
	UINT8* m_cbvDataBegin;
	UINT m_srvDescSize;

	AnimationConstantBuffer m_instanceData;
	ComPtr<ID3D12Resource> m_StructuredBuffer;
	UINT8* m_StructuredBufferDataBegin;

	ComPtr<ID3D12Resource> m_vertexBuffer;
	ComPtr<ID3D12Resource> m_vertexBufferUpload;
	ComPtr<ID3D12Resource> m_indexBuffer;
	ComPtr<ID3D12Resource> m_indexBufferUpload;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW	m_indexBufferView;

	XMFLOAT4X4 m_worldTransform;

	std::vector<FbxMeshData> meshes;

	DirectionalLight m_light;

	vector<AnimClipInfo> m_animClips;
	vector<BoneInfo> m_bones;

	std::shared_ptr<Texture> m_TColor;
	std::shared_ptr<Texture> m_TNormal;

	vector<XMMATRIX> boneTransforms;
};