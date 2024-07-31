#pragma once
#include "stdafx.h"
#include "Structures.h"
#include "Block.h"
#include "Cube.h"


using namespace graphics;

static const int CHUNK_SIZE = 16;

class Chunk
{
public: 
	Chunk(Graphics* renderer);
	~Chunk();

	BlockType GetBlock(int x, int y, int z) const;

	void GenerateChunk();
	void Draw(ComPtr<ID3D12GraphicsCommandList>& m_commandList, XMFLOAT4X4& viewproj, XMFLOAT4& eye);

private:
	std::vector<Cube> m_blocks;
	Cube m_block;
	std::vector<InstanceBuffer> instanceData;

	Graphics* m_renderer;
};