#pragma once
#include "Renderer.h"
#include "stdafx.h"
#include "Structures.h"
#include "Object.h"
#include "FBXLoader.h"
#include "DirectionalLight.h"

using namespace graphics;

struct CharacterConstantBuffer
{
	int edgeTessellationFactor = 1;
	int insideTessellationFactor = 1;

	XMMATRIX BoneTransforms[96];
};

class Achates : public Object
{
public:
	Achates(Graphics* renderer);
	~Achates();

	void Draw(ComPtr<ID3D12GraphicsCommandList> m_commandList, XMFLOAT4X4 viewproj, XMFLOAT4 eye);

	void ClearUnusedUploadBuffersAfterInit();

	void SetIsWireframe(bool isWireframe) { this->isWireframe = isWireframe; }


	vector<shared_ptr<FbxBoneInfo>> m_boneInfos;
	int m_edgetesFactor = 1;
	int m_insidetesFactor = 1;
	
private:
	void InitPipeline(Graphics* Renderer);
	void InitPipelineWireframe(Graphics* Renderer);

	void CreateDescriptorHeap(Graphics* Renderer);

	void CreateConstantBuffer(Graphics* Renderer);

	void LoadFBXModel(Graphics* Renderer, string path);
	void LoadFBXAnimation(Graphics* Renderer, string path);


	
	XMMATRIX GetMatrix(FbxAMatrix& mat);

	ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	ID3D12Resource* m_uploadHeap;

	UINT m_width;
	UINT m_height;

	bool isWireframe = false;
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12PipelineState> m_pipelineStateWireframe;
	ComPtr<ID3D12RootSignature> m_rootSignature;

	// Constant Buffer : World, View, Projection
	ID3D12Resource* m_CBV;
	ConstantBuffer m_constantBufferData;
	UINT8* m_cbvDataBegin;

	// Constant Buffer : Character 
	ID3D12Resource* m_CBV2;
	CharacterConstantBuffer m_constantBuffer2Data;
	UINT8* m_cbv2DataBegin;
	UINT m_srvDescSize;

	// Constant Buffer : Bone Offset
	ID3D12Resource* m_cboffset;
	CharacterConstantBuffer m_cboffsetData;
	UINT8* m_cboffsetBegin;

	ID3D12Resource* m_vertexBuffer;
	ID3D12Resource* m_vertexBufferUpload;
	ID3D12Resource* m_indexBuffer;
	ID3D12Resource* m_indexBufferUpload;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW	m_indexBufferView;

	XMFLOAT4X4 m_worldTransform;

	vector<FbxMeshData> meshes;

	DirectionalLight m_light;

	vector<AnimClipInfo> m_animClips;
	vector<BoneInfo> m_bones;

};