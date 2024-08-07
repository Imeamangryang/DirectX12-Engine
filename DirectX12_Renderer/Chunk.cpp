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

Chunk::Chunk(Graphics* renderer)
{
	m_renderer = renderer;

	// 여러 블록을 초기화
	for (int i = 0; i < CHUNK_DISTANCE * CHUNK_DISTANCE; ++i) {
		m_blocks.push_back(Cube(renderer));
	}

	// 여러 블록을 초기화
	for (int i = 0; i < CHUNK_DISTANCE; ++i) {
		for (int j = 0; j < CHUNK_DISTANCE; ++j) {
			GenerateChunk(i, j);
			m_blocks[i * CHUNK_DISTANCE + j].CreateInstanceBuffer(m_renderer, instanceData);
		}
	}
}

Chunk::~Chunk()
{
}

BlockType Chunk::GetBlock(float x, float y, int z)
{
	// Simplex Noise
	float continentalness = continent.fractal(4, x, y);
	float erosioness = erosion.fractal(5, x, y);
	float peaksValleyness = peaksValleys.fractal(4, x, y);

	// Perlin Noise
	//float continentalness = Pcontinent.noise(x, y, 4);
	//float erosioness = Perosion.noise(x, y, 5);
	//float peaksValleyness = PpeaksValleys.noise(x, y, 4);

	float height = 0;

	height = getSplineValue(continentalness, ContinentalnessNodes);
	height += getSplineValue(erosioness, ErosionNodes);
	height += getSplineValue(peaksValleyness, PeakValleysNodes);

	//height = 108 * 0.3 * (continentalness + 1) / 2;

	//if (height >= 128 * 0.95) height = 128 * 0.95;
	if (height <= 20) height = 20;

	//float surfaceZ = 30 + continentalness * 20;

	if (z < height)
	{
		return BlockType::Dirt;
	}
	else
	{
		return BlockType::Air;
	}
}

float Chunk::getSplineValue(float value, std::map<float, float> nodes)
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
			for (int z = 0; z < 255; z++)
			{
				XMFLOAT4X4 world = MathHelper::Identity4x4();
				XMStoreFloat4x4(&world, XMMatrixTranslation(x * 4.0f + (regionx * 64), y * 4.0f + (regiony * 64), z * 4.0f));
				data.world = world;
				data.blockType = (UINT)GetBlock((x + (regionx * 16)) * 0.016f, (y + (regiony * 16)) * 0.016f, z);
				data.isvisible = true;
				if (data.blockType != (UINT)BlockType::Air) {
					instanceData.push_back(data);
				}
			}
		}
	}
}

void Chunk::Draw(ComPtr<ID3D12GraphicsCommandList>& m_commandList, XMFLOAT4X4& viewproj, XMFLOAT4& eye)
{
	for (auto& block : m_blocks) {
		block.Draw(m_commandList, viewproj, eye);
	}
}



