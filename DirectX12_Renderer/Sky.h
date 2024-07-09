#pragma once

#include "Renderer.h"
#include "stdafx.h"
#include "Structures.h"

using namespace graphics;

class Sky
{
public:
	Sky(Graphics* renderer);
	~Sky();

	void Draw3D(ComPtr<ID3D12GraphicsCommandList> m_commandList, XMFLOAT4X4 viewproj, XMFLOAT4 eye);

	void ClearUnusedUploadBuffersAfterInit();

private:
	void InitPipeline3D(Graphics* Renderer);

	void CreateConstantBuffer(Graphics* Renderer);

	void LoadHeightMap(Graphics* Renderer, const wchar_t* displacementmap, const wchar_t* colormap);

	void CreateGeosphere(Graphics* Renderer, float radius, UINT numSubdivisions);

	ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	ID3D12Resource* m_uploadHeap;
	UINT m_width;
	UINT m_height;

	ComPtr<ID3D12PipelineState> m_pipelineState3D;
	ComPtr<ID3D12RootSignature> m_rootSignature3D;
	ID3D12Resource* m_CBV;
	SkyConstantBuffer m_constantBufferData;
	UINT8* m_cbvDataBegin;
	UINT m_srvDescSize;

	ID3D12Resource* m_vertexBuffer;
	ID3D12Resource* m_vertexBufferUpload;
	ID3D12Resource* m_indexBuffer;
	ID3D12Resource* m_indexBufferUpload;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW	m_indexBufferView;
	UINT m_indexcount;

	XMFLOAT4X4 m_worldTransform;
};