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

float Terrain::heightAt(float x, float y) const
{
	return _heightMap.get(x, y)*50;
}

vec3f Terrain::normalAt(float x, float y) const
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


bool Terrain::contains(float x, float y) const
{
	return _heightMap.contains(x,y);
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

	return SAVEIMAGE(fileName, [this,min,max](unsigned x, unsigned y){ return (heightAt(x,y)-min)/(max-min)*255; }, _heightMap.size());
}

void Terrain::init()
{
	auto g = [this](unsigned x, unsigned y) -> float{
		PerlinMultiOctave::Octaves octaves(0.15, 0.02, 3, 0.3);
		octaves.addOctave({0.03,0.09});
		static PerlinMultiOctave g(_size.X, _size.Y, octaves,_seed);
		return g.val(x, y)*2;
	};
	_heightMap.generate(g);
	dumpHeightmapToImage("heightmap.bmp");
}
