#include "Chunk.h"

std::map<float, float> ContinentalnessNodes = {
	{-1.0, 42},
	{-0.4, 42},
	{-0.5, 42 + 15},
	{0.0, 42 + 15},
	{0.1, 42 + 25},
	{0.2, 42 + 40},
	{0.5, 42 + 45},
	{1.0, 42 + 50},
};

std::map<float, float> ErosionNodes = {
	 {-1.0, 25},
	 {-0.7, 10},
	 {-0.6, 15},
	 {-0.2, 12},
	 {0.2, -10},
	 {0.4,-20},
	 {0.6,-20},
	 {0.65,-10},
	 {0.7,-10},
	 {0.75, -20},
	 {1.0, -25}
};

std::map<float, float> PeakValleysNodes = {
	{-1.0, -5},
	{-0.8, -3},
	{-0.5, -1},
	{0.0, 1.0},
	{0.5, 8},
	{0.7, 12},
	{1.0, 10},
};

Chunk::Chunk(Graphics* renderer, XMFLOAT4 cameraPosition)
{
	m_renderer = renderer;

	// 카메라 위치를 기준으로 청크를 생성
	int cameraChunkX = static_cast<int>(cameraPosition.x / (CHUNK_SIZE * 4.0f));
	int cameraChunkY = static_cast<int>(cameraPosition.y / (CHUNK_SIZE * 4.0f));

	for (int dx = -CHUNK_DISTANCE * 5; dx <= CHUNK_DISTANCE * 5; ++dx)
	{
		for (int dy = -CHUNK_DISTANCE * 5; dy <= CHUNK_DISTANCE * 5; ++dy)
		{
			int chunkX = cameraChunkX + dx;
			int chunkY = cameraChunkY + dy;

			m_chunkMap.push_back(std::make_pair(chunkX, chunkY));

			// Draw할 Chunk 생성
			m_blocks.push_back(Cube(m_renderer));
			GenerateChunk(chunkX, chunkY);
			m_blocks.back().CreateInstanceBuffer(m_renderer, instanceData);
		}
	}
	//for (int dx = -CHUNK_DISTANCE; dx <= CHUNK_DISTANCE; ++dx)
	//{
	//	for (int dy = -CHUNK_DISTANCE; dy <= CHUNK_DISTANCE; ++dy)
	//	{
	//		int chunkX = cameraChunkX + dx;
	//		int chunkY = cameraChunkY + dy;

	//		m_chunkMap.push_back(std::make_pair(chunkX, chunkY));

	//		// Draw할 Chunk 생성
	//		m_blocks.push_back(Cube(m_renderer));
	//		GenerateChunk(chunkX, chunkY);
	//		m_blocks.back().CreateInstanceBuffer(m_renderer, instanceData);
	//	}
	//}
}

Chunk::~Chunk()
{
}

BlockType Chunk::GetBlock(float x, float y, int z, int height)
{
	if (z < height)
	{
		return BlockType::Stone;
	}
	else if (z == height)
	{
		return BlockType::Dirt;
	}
	else if (z < 30) {
		return BlockType::Water;
	}
	else
	{
		return BlockType::Air;
	}
}

int Chunk::GetHeight(float x, float y)
{
	float continentalness = continent.fractal(4, x, y);
	float erosioness = erosion.fractal(5, x, y);
	float peaksValleyness = peaksValleys.fractal(4, x, y);

	// Perlin Noise
	//float continentalness = Pcontinent.noise(x, y, 4);
	//float erosioness = Perosion.noise(x, y, 5);
	//float peaksValleyness = PpeaksValleys.noise(x, y, 4);

	float height = 0;

	height = GetSplineValue(continentalness, ContinentalnessNodes);
	height += GetSplineValue(erosioness, ErosionNodes);
	height += GetSplineValue(peaksValleyness, PeakValleysNodes);

	return (int)height;
}

float Chunk::GetSplineValue(float value, std::map<float, float> nodes)
{
	float min = 0;
	float max = 0;
	float start = 0;
	float end = 0;
	for (auto node : nodes)
	{
		if (node.first < value)
		{
			start = node.first;
			min = node.second;
		}
		else
		{
			end = node.first;
			max = node.second;
			break;
		}
	}
	if (max == min)
	{
		return max;
	}

	float k = (max - min) / (end - start);
	float b = max - end * k;

	return value * k + b;
}

void Chunk::GenerateChunk(int regionx, int regiony)
{
	instanceData.clear();
	InstanceBuffer data;

	for (int x = 0; x < CHUNK_SIZE; x++)
	{
		for (int y = 0; y < CHUNK_SIZE; y++)
		{
			int ChunkHeight = GetHeight((x + (regionx * 16)) * 0.016f, (y + (regiony * 16)) * 0.016f);
			if (ChunkHeight < 30) {
				for (int z = 30; z > ChunkHeight - 5; z--)
				{
					XMFLOAT4X4 world = MathHelper::Identity4x4();
					XMStoreFloat4x4(&world, XMMatrixTranslation(x * 4.0f + (regionx * 64), y * 4.0f + (regiony * 64), z * 4.0f));
					data.world = world;
					data.blockType = (UINT)GetBlock((x + (regionx * 16)) * 0.06f, (y + (regiony * 16)) * 0.06f, z, ChunkHeight);
					data.isvisible = false;
					if (data.blockType != (UINT)BlockType::Air) {
						instanceData.push_back(data);
					}
				}
			}
			else {
				for (int z = ChunkHeight; z > ChunkHeight - 5; z--)
				{
					XMFLOAT4X4 world = MathHelper::Identity4x4();
					XMStoreFloat4x4(&world, XMMatrixTranslation(x * 4.0f + (regionx * 64), y * 4.0f + (regiony * 64), z * 4.0f));
					data.world = world;
					data.blockType = (UINT)GetBlock((x + (regionx * 16)) * 0.06f, (y + (regiony * 16)) * 0.06f, z, ChunkHeight);
					data.isvisible = false;
					if (data.blockType != (UINT)BlockType::Air) {
						instanceData.push_back(data);
					}
				}
			}
			
			//for (int z = 0; z < 255; z++)
			//{
			//	int ChunkHeight = GetHeight((x + (regionx * 16)) * 0.016f, (y + (regiony * 16)) * 0.016f);
			//	XMFLOAT4X4 world = MathHelper::Identity4x4();
			//	XMStoreFloat4x4(&world, XMMatrixTranslation(x * 4.0f + (regionx * 64), y * 4.0f + (regiony * 64), z * 4.0f));
			//	data.world = world;
			//	data.blockType = (UINT)GetBlock((x + (regionx * 16)) * 0.016f, (y + (regiony * 16)) * 0.016f, z, ChunkHeight);
			//	data.isvisible = true;
			//	if (data.blockType != (UINT)BlockType::Air) {
			//		instanceData.push_back(data);
			//	}
			//}
		}
	}
}

void Chunk::Draw(ComPtr<ID3D12GraphicsCommandList>& m_commandList, XMFLOAT4X4& viewproj, XMFLOAT4& eye)
{
	for (auto& block : m_blocks) {
		block.Draw(m_commandList, viewproj, eye);
	}
}

void Chunk::UpdateChunks(XMFLOAT4 cameraPosition)
{
	// 카메라 위치를 기준으로 청크를 생성
	int cameraChunkX = static_cast<int>(cameraPosition.x / (CHUNK_SIZE * 2));
	int cameraChunkY = static_cast<int>(cameraPosition.y / (CHUNK_SIZE * 2));

	for (int dx = -CHUNK_DISTANCE; dx <= CHUNK_DISTANCE; ++dx)
	{
		for (int dy = -CHUNK_DISTANCE; dy <= CHUNK_DISTANCE; ++dy)
		{
			int chunkX = cameraChunkX + dx;
			int chunkY = cameraChunkY + dy;

			// 중복 청크 생성 방지
			if (std::find(m_chunkMap.begin(), m_chunkMap.end(), std::make_pair(chunkX, chunkY)) == m_chunkMap.end())
			{
				m_chunkMap.push_back(std::make_pair(chunkX, chunkY));

				// Draw할 Chunk 생성
				m_blocks.push_back(Cube(m_renderer));
				GenerateChunk(chunkX, chunkY);
				m_blocks.back().CreateInstanceBuffer(m_renderer, instanceData);
			}
		}
	}
}



