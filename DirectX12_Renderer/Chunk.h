#pragma once
#include "stdafx.h"
#include "Structures.h"
#include "Block.h"
#include "Cube.h"
#include "PerlinNoise.h"
#include "SimplexNoise.h"
#include <map>


using namespace graphics;

static const int CHUNK_SIZE = 16;
static const int CHUNK_DISTANCE = 2;

class Chunk
{
public: 
	Chunk(Graphics* renderer, XMFLOAT4 cameraPosition);
	~Chunk();

	BlockType GetBlock(float x, float y, int z);
	float getSplineValue(float value, std::map<float, float> nodes);

	void GenerateChunk(int regionx, int regiony);
	void Draw(ComPtr<ID3D12GraphicsCommandList>& m_commandList, XMFLOAT4X4& viewproj, XMFLOAT4& eye);
	void UpdateChunks(XMFLOAT4 cameraPosition);

	float GetContinentNoise(float x, float y) { return continent.fractal(4, x * 0.016f, y * 0.016f); }
	float GetErosionNoise(float x, float y) { return erosion.fractal(5, x * 0.016f, y * 0.016f); }
	float GetPeaksValleysNoise(float x, float y) { return peaksValleys.fractal(4, x * 0.016f, y * 0.016f); }

	//float GetContinentNoise(float x, float y) { return continent.noise(x * 0.016f, y * 0.016f, 4); }
	//float GetErosionNoise(float x, float y) { return erosion.noise(x * 0.016f, y * 0.016f, 5); }
	//float GetPeaksValleysNoise(float x, float y) { return peaksValleys.noise(x * 0.016f, y * 0.016f, 4); }

private:
	std::vector<Cube> m_blocks;
	std::vector<InstanceBuffer> instanceData;
	std::vector<std::pair<int, int>> m_chunkMap;

	SimplexNoise continent = SimplexNoise(0.05f, 1, 2, 0.5f);
	SimplexNoise erosion = SimplexNoise(0.02f, 1, 2, 0.5f);
	SimplexNoise peaksValleys = SimplexNoise(0.6f, 1, 2, 0.5f);

	PerlinNoise Pcontinent = PerlinNoise(200, 0.05f, 1, 2, 0.5f);
	PerlinNoise Perosion = PerlinNoise(200, 0.02f, 1, 2, 0.5f);
	PerlinNoise PpeaksValleys = PerlinNoise(200, 0.6f, 1, 2, 0.5f);


	Graphics* m_renderer;
};