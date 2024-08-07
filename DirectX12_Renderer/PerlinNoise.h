#pragma once
#include "stdafx.h"

class PerlinNoise
{
public:
	PerlinNoise(unsigned int seed = std::default_random_engine::default_seed,
		float frequency = 1.0f,
		float amplitude = 1.0f,
		float lacunarity = 2.0f,
		float persistence = 0.5f
		);

	double noise(double x, double y, int octaves);

private:
	float mFrequency;   ///< Frequency ("width") of the first octave of noise (default to 1.0)
	float mAmplitude;   ///< Amplitude ("height") of the first octave of noise (default to 1.0)
	float mLacunarity;  ///< Lacunarity specifies the frequency multiplier between successive octaves (default to 2.0).
	float mPersistence; ///< Persistence is the loss of amplitude between successive octaves (usually 1/lacunarity)
	std::vector<int> p;

	double singleNoise(double x, double y, double z);

	double fade(double t);

	double lerp(double t, double a, double b);

	double grad(int hash, double x, double y, double z);

};