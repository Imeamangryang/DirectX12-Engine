#pragma once
#include "stdafx.h"
#include "Structures.h"
#include "Object.h"
#include "DirectionalLight.h"
#include "Texture.h"

using namespace graphics;

struct ParticleVertex
{
	XMFLOAT4 color;
};

class Particle : public Object
{
public:
	Particle(Graphics* renderer);
	~Particle();

	void Draw(ComPtr<ID3D12GraphicsCommandList> m_commandList, XMFLOAT4X4 viewproj, XMFLOAT4 eye);

	void SetIsWireframe(bool isWireframe) { this->isWireframe = isWireframe; }

private:
	void CreateDescriptorHeap(Graphics* Renderer);

	void InitPipeline(Graphics* Renderer);

	void LoadMesh(Graphics* Renderer);


	ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	UINT m_srvDescSize;

	bool isWireframe = false;
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12PipelineState> m_computePipelineState;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12RootSignature> m_computeRootSignature;

	ComPtr<ID3D12Resource> m_CBV;
	ConstantBuffer m_constantBufferData;
	UINT8* m_cbvDataBegin;

	std::vector<InstanceBuffer> m_instanceData;
	ComPtr<ID3D12Resource> m_StructuredBuffer;
	UINT8* m_StructuredBufferDataBegin;
	UINT instanceCount;

	ComPtr<ID3D12Resource> m_vertexBuffer;
	ComPtr<ID3D12Resource> m_vertexBufferUpload;
	ComPtr<ID3D12Resource> m_indexBuffer;
	ComPtr<ID3D12Resource> m_indexBufferUpload;

	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW	m_indexBufferView;

	XMFLOAT4X4 m_worldTransform;

	DirectionalLight m_light;

	static const UINT ParticleCount = 10000;
};