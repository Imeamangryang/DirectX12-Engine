#include "Chunk.h"

Chunk::Chunk(Graphics* renderer) :
	m_block(renderer)
{
	m_renderer = renderer;
	GenerateChunk();
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
	InstanceBuffer data;
	for (int x = 0; x < CHUNK_SIZE; x++)
	{
		for (int y = 0; y < CHUNK_SIZE; y++)
		{
			for (int z = 0; z < 10; z++)
			{
				XMFLOAT4X4 world = MathHelper::Identity4x4();
				XMStoreFloat4x4(&world, XMMatrixTranslation(x * 4.0f, y * 4.0f, z * 4.0f));
				data.world = world;
				instanceData.push_back(data);
			}
		}
	}
	m_block.CreateInstanceBuffer(m_renderer, instanceData);

}

void Chunk::Draw(ComPtr<ID3D12GraphicsCommandList>& m_commandList, XMFLOAT4X4& viewproj, XMFLOAT4& eye)
{
	m_block.Draw(m_commandList, viewproj, eye, instanceData.size());
}


