#pragma once
#include "Renderer.h"
#include "stdafx.h"
#include "Structures.h"
#include "Object.h"
#include "FBXLoader.h"
#include "DirectionalLight.h"

using namespace graphics;

struct IndexBufferInfo
{
	ComPtr<ID3D12Resource>		buffer;
	D3D12_INDEX_BUFFER_VIEW		bufferView;
	DXGI_FORMAT					format;
	UINT						count;
};

struct KeyFrameInfo
{
	double	time;
	UINT	frame;
	XMFLOAT3	scale;
	XMFLOAT4	rotation;
	XMFLOAT3	translate;
};

struct BoneInfo
{
	string					boneName;
	UINT					parentIdx;
	XMMATRIX				matOffset;
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
	int edgeTessellationFactor = 1;
	int insideTessellationFactor = 1;
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

	ID3D12Resource* m_CBV2;
	CharacterConstantBuffer m_constantBuffer2Data;
	UINT8* m_cbv2DataBegin;
	UINT m_srvDescSize;

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