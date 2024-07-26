#pragma once
#include "stdafx.h"
#include "Structures.h"
#include "Block.h"

using namespace graphics;

static const int CHUNK_SIZE = 16;

class Chunk
{
public: 
	Chunk(Graphics* renderer);
	~Chunk() = default;

	Block GetBlock(int x, int y, int z) const;

	void GenerateChunk();
	void Draw(ComPtr<ID3D12GraphicsCommandList> m_commandList, XMFLOAT4X4 viewproj, XMFLOAT4 eye);

	void ClearUnusedUploadBuffersAfterInit();

private:
	void CreateDescriptorHeap(Graphics* Renderer);
	void InitPipeline(Graphics* Renderer);
	void LoadChunk(Graphics* Renderer);

	std::vector<Block> m_blocks;

	ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	ID3D12Resource* m_uploadHeap;

	bool isWireframe = false;
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12PipelineState> m_pipelineStateWireframe;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ID3D12Resource* m_CBV;
	ConstantBuffer m_constantBufferData;
	UINT8* m_cbvDataBegin;
	UINT m_srvDescSize;

	ID3D12Resource* m_vertexBuffer;
	ID3D12Resource* m_vertexBufferUpload;
	ID3D12Resource* m_indexBuffer;
	ID3D12Resource* m_indexBufferUpload;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW	m_indexBufferView;
};