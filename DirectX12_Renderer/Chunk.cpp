#include "Chunk.h"

Chunk::Chunk(Graphics* renderer)
{
	
}

Block Chunk::GetBlock(int x, int y, int z) const
{
	return Block(x, y, z, BlockType::Air);
}

void Chunk::GenerateChunk()
{
	for (int x = 0; x < CHUNK_SIZE; x++)
	{
		for (int y = 0; y < CHUNK_SIZE; y++)
		{
			for (int z = 0; z < 1; z++)
			{
				Block block = GetBlock(x, y, z);
				m_blocks.push_back(block);
			}
		}
	}
}

void Chunk::Draw(ComPtr<ID3D12GraphicsCommandList> m_commandList, XMFLOAT4X4 viewproj, XMFLOAT4 eye)
{
	for (auto& block : m_blocks)
	{
		UINT cbvIndex = (UINT)m_blocks.size();
		CD3DX12_GPU_DESCRIPTOR_HANDLE cbHandle(m_srvHeap->GetGPUDescriptorHandleForHeapStart());
		cbHandle.Offset(cbvIndex, m_srvDescSize);

		m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
		m_commandList->IASetIndexBuffer(&m_indexBufferView);
		
		m_commandList->DrawIndexedInstanced(1, 0, 0, 0, 0 );
	}
}

void Chunk::ClearUnusedUploadBuffersAfterInit()
{
}
