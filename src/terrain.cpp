#include "terrain.hpp"
#include "perlinMultiOctave.hpp"
#include <iostream>

Terrain::Terrain(vec2u size, unsigned seed): _size{size}, _heightMap(size), _seed{seed}
{
	init();
}

vec2u Terrain::size() const
{
	return _size;
}

float Terrain::heightAt(unsigned x, unsigned y) const
{
	return _heightMap.get(x, y)*50;
}

vec3f Terrain::normalAt(unsigned x, unsigned y) const
{
	return _heightMap.getNormal(x,y);
}

void Terrain::setSeed(unsigned seed)
{
	_seed = seed;
	init();
}

unsigned Terrain::getSeed()
{
	return _seed;
}

bool Terrain::dumpHeightmapToImage(std::string fileName)
{
	float min, max;
	min = max = heightAt(0,0);
	for(unsigned y = 0; y < _heightMap.size().Y; y++)
		for(unsigned x = 0; x < _heightMap.size().X; x++) {
			float v = heightAt(x,y);
			if(v < min)
				min = v;
			if(v > max)
				max = v;
		}
	std::cout << max << " " << min << std::endl;

	return SAVEIMAGE(fileName, [this,min,max](unsigned x, unsigned y){ return (heightAt(x,y)-min)/(max-min)*255; }, _heightMap.size());
}

void Terrain::init()
{
	auto g = [this](unsigned x, unsigned y) -> float{
		static PerlinMultiOctave g(_size.X, _size.Y, PerlinMultiOctave::Octaves(0.4, 0.01, 3, 0.2),_seed);
		return g.val(x, y)*2;
	};
	_heightMap.generate(g);
	dumpHeightmapToImage("heightmap.bmp");
}
