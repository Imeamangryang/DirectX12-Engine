#pragma once
#include "stdafx.h"
#include "Structures.h"

using namespace graphics;

class Block
{
public:
	Block(int x, int y, int z, BlockType blocktypes);
	~Block() = default;



private:
	XMFLOAT3 m_position;

	vector<Vertex> vertices;
	vector<size_t> indices;
};