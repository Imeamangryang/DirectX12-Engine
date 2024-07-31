#pragma once
#include "stdafx.h"

class PerlinNoise 
{
public:
	PerlinNoise(unsigned int seed = std::default_random_engine::default_seed);

    // 옥타브를 고려한 2D Perlin Noise 생성
	double noise(double x, double y, int octaves);

private:
    std::vector<int> p;

	double singleNoise(double x, double y, double z);

	double fade(double t);

	double lerp(double t, double a, double b);

	double grad(int hash, double x, double y, double z);

};