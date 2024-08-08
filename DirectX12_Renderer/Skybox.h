#pragma once
#include "stdafx.h"
#include "Structures.h"
#include "Object.h"
#include "Texture.h"

using namespace graphics;

class Skybox : public Object
{
public:
	Skybox(Graphics* renderer);
	~Skybox();

	void Draw(ComPtr<ID3D12GraphicsCommandList> m_commandList, XMFLOAT4X4 viewproj, XMFLOAT4 eye);

private:

	void InitPipeline();

	void CreateDescriptorHeap();

	void LoadMesh();

	Graphics* m_renderer;
	ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	UINT m_srvDescSize;

	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12RootSignature> m_rootSignature;

	ComPtr<ID3D12Resource> m_vertexBuffer;
	ComPtr<ID3D12Resource> m_vertexBufferUpload;
	ComPtr<ID3D12Resource> m_indexBuffer;
	ComPtr<ID3D12Resource> m_indexBufferUpload;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW	m_indexBufferView;
	UINT m_indexcount;

	ComPtr<ID3D12Resource> m_constantBuffer;
	SkyConstantBuffer m_constantBufferData;
	UINT8* m_constantBufferDataBegin;

	std::shared_ptr<Texture> m_Tsky;
	XMFLOAT4X4 m_worldTransform = MathHelper::Identity4x4();
};