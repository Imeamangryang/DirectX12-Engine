#include "Chunk.h"

Chunk::Chunk(Graphics* renderer) :
	m_block(0, 0, 0, BlockType::Stone, renderer)
{
	m_renderer = renderer;
}

Chunk::~Chunk()
{
}

BlockType Chunk::GetBlock(int x, int y, int z) const
{
	return BlockType::Stone;
}

void Chunk::GenerateChunk()
{
	for (int x = 0; x < CHUNK_SIZE; x++)
	{
		for (int y = 0; y < CHUNK_SIZE; y++)
		{
			for (int z = 0; z < 1; z++)
			{
				m_blocks.push_back(std::make_pair(BlockType::Stone, XMFLOAT3(x, y, z)));
			}
		}
	}
}

void Chunk::Draw(ComPtr<ID3D12GraphicsCommandList>& m_commandList, XMFLOAT4X4& viewproj, XMFLOAT4& eye)
{
	for(auto& block : m_blocks)
	{
		//XMFLOAT4X4 world = MathHelper::Identity4x4();
		//XMStoreFloat4x4(&world, XMMatrixTranslation(block.second.x, block.second.y, block.second.z));
		//m_block.SetWorldTransform(world);
		//m_block.SetBlockType(block.first);
		
	}
	m_block.Draw(m_commandList, viewproj, eye);
}

void Chunk::ClearUnusedUploadBuffersAfterInit()
{
	m_block.ClearUnusedUploadBuffersAfterInit();
}


