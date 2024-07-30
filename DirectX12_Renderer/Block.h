#pragma once
#include "stdafx.h"
#include "Structures.h"
#include "Texture.h"
#include "DirectionalLight.h"

using namespace graphics;

class Block
{
public:
	Block(int x, int y, int z, BlockType blocktypes, Graphics* renderer);
	~Block();

	// 이동 할당 연산자
	Block& operator=(Block&& other) noexcept {
		if (this != &other) {
			m_position.x = other.m_position.x;
			m_position.y = other.m_position.y;
			m_position.z = other.m_position.z;
			m_blockType = other.m_blockType;
			m_renderer = other.m_renderer;
			other.m_renderer = nullptr;
		}
		return *this;
	}

	XMFLOAT3 GetPosition() const { return m_position; }
	BlockType GetBlockType() const { return m_blockType; }

	void Draw(ComPtr<ID3D12GraphicsCommandList> m_commandList, XMFLOAT4X4 viewproj, XMFLOAT4 eye);
	void SetWorldTransform(XMFLOAT4X4 worldTransform) { m_worldTransform = worldTransform; }
	void SetBlockType(BlockType blockType) { m_blockType = blockType; }

	void ClearUnusedUploadBuffersAfterInit();

	UINT m_indexcount;
	UINT m_vertexcount;

private:
	void CreateDescriptorHeap();
	void InitPipeline();
	void LoadMesh();

	std::vector<Block> m_blocks;

	ComPtr<ID3D12DescriptorHeap> m_srvHeap;

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

	XMFLOAT4X4 m_worldTransform;

	DirectionalLight m_light;

	std::unique_ptr<Texture> m_Texture;

	XMFLOAT3 m_position;
	BlockType m_blockType;

	Graphics* m_renderer;

	vector<Vertex> vertices;
	vector<size_t> indices;
};