#include "perlinMultiOctave.hpp"
#include <cmath>
#include <cassert>
#include <iostream>
#include <algorithm>


PerlinMultiOctave::Octaves::Octaves(float maxFrequency, float amplitude, unsigned octavesC, float persistence)
{
	assert(persistence <= 1);
	float a = amplitude;
	float f = maxFrequency;
	for(unsigned i=0; i < octavesC; i++)
	{
		push_back({f,a});
		f *= persistence;
		a *= 1/persistence;
	}
}

void PerlinMultiOctave::Octaves::addOctave(Octave o) 
{
	push_back(o);
}

float PerlinMultiOctave::Octaves::maxFrequency() const
{
	return std::max_element(begin(), end(), [](const Octave& lhs, const Octave& rhs) { return lhs.frequency < rhs.frequency; })->frequency;
}


PerlinMultiOctave::PerlinMultiOctave(unsigned sizeX,
	 	unsigned sizeY, Octaves octaves, unsigned seed):
	_perlin{(unsigned)std::ceil(sizeX*octaves.maxFrequency()*1.1), 
					(unsigned)std::ceil(sizeY*octaves.maxFrequency()*1.1), seed},
					_octaves{octaves}
{
}

float PerlinMultiOctave::val(unsigned x, unsigned y)
{
	float r = 0;
	for(Octave& o : _octaves)
		r += o.amplitude*_perlin.val(x*o.frequency, y*o.frequency);
	return r;
}

void PerlinMultiOctave::setSeed(unsigned seed) {
	_perlin.setSeed(seed);
}

unsigned PerlinMultiOctave::getSeed() {
	return _perlin.getSeed();
}
