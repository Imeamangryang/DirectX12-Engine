#include "Chunk.h"

Chunk::Chunk(Graphics* renderer) :
	m_block(renderer)
{
	m_renderer = renderer;
	GenerateChunk();

	std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

Chunk::~Chunk()
{
}

BlockType Chunk::GetBlock(int x, int y, int z) const
{
	float frequency = 0.1f;
	float amplitude = 10.0f;

	float offset_x = std::sin(x * frequency) * amplitude;
	float offset_y = std::cos(y * frequency) * amplitude;

	float surfaceZ = 50 + offset_x + offset_y;

	float stoneZ = 52;

	if (z < surfaceZ)
	{
		return BlockType::Dirt;
	}
	else if (z < stoneZ)
	{
		return BlockType::Stone;
	}
	else
	{
		return BlockType::Air;
	}
}

void Chunk::GenerateChunk()
{
	InstanceBuffer data;

	for (int x = 0; x < CHUNK_SIZE * 5; x++)
	{
		for (int y = 0; y < CHUNK_SIZE * 5; y++)
		{
			for (int z = 0; z < 255; z++)
			{
				XMFLOAT4X4 world = MathHelper::Identity4x4();
				XMStoreFloat4x4(&world, XMMatrixTranslation(x * 4.0f, y * 4.0f, z * 4.0f));
				data.world = world;
				data.blockType = (UINT)GetBlock(x, y, z);
				if (data.blockType != (UINT)BlockType::Air) {
					instanceData.push_back(data);
				}
			}
		}
	}
	m_block.CreateInstanceBuffer(m_renderer, instanceData);

}

void Chunk::Draw(ComPtr<ID3D12GraphicsCommandList>& m_commandList, XMFLOAT4X4& viewproj, XMFLOAT4& eye)
{
	m_block.Draw(m_commandList, viewproj, eye, instanceData.size());
}


