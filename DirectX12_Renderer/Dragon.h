#pragma once
#include "Renderer.h"
#include "stdafx.h"
#include "Structures.h"
#include "Object.h"
#include "FBXLoader.h"
#include "DirectionalLight.h"

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
	//int boneCount;
	//int CurrentFrame;
	//int nextFrame;

	int edgeTessellationFactor = 1;
	int insideTessellationFactor = 1;
	XMMATRIX BoneTransforms[200];
};

class Dragon : public Object
{
public:
	Dragon(Graphics* renderer);
	~Dragon();

	void Draw(ComPtr<ID3D12GraphicsCommandList> m_commandList, XMFLOAT4X4 viewproj, XMFLOAT4 eye);

	void ClearUnusedUploadBuffersAfterInit();

	void SetIsWireframe(bool isWireframe) { this->isWireframe = isWireframe; }

	static bool Equal(float f0, float f1);

	vector<shared_ptr<FbxBoneInfo>> m_boneInfos;

	int m_edgetesFactor1 = 1;
	int m_edgetesFactor2 = 1;
	int m_edgetesFactor3 = 1;
	int m_insidetesFactor = 1;

private:
	void InitPipeline(Graphics* Renderer);

	void CreateDescriptorHeap(Graphics* Renderer);

	void CreateConstantBuffer(Graphics* Renderer);

	void LoadFBXModel(Graphics* Renderer, string path);
	
	void CalcAENIndices(const FbxMeshInfo& meshinfo, vector<uint32_t>& indices);




	XMFLOAT4X4 GetMatrix(FbxAMatrix& mat);


	ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	ID3D12Resource* m_uploadHeap;

	UINT m_width;
	UINT m_height;

	bool isWireframe = false;
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12PipelineState> m_pipelineStateWireframe;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ID3D12Resource* m_CBV;
	ConstantBuffer m_constantBufferData;
	UINT8* m_cbvDataBegin;
	UINT m_srvDescSize;

	ID3D12Resource* m_CbAnim;
	CharacterConstantBuffer m_CbAnimData;
	UINT8* m_CbAnimDataBegin;

	ID3D12Resource* m_vertexBuffer;
	ID3D12Resource* m_vertexBufferUpload;
	ID3D12Resource* m_indexBuffer;
	ID3D12Resource* m_indexBufferUpload;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW	m_indexBufferView;

	XMFLOAT4X4 m_worldTransform;

	std::vector<FbxMeshData> meshes;

	DirectionalLight m_light;

	vector<AnimClipInfo> m_animClips;
	vector<BoneInfo> m_bones;
};