#include <cmath>
#include <cassert>
#include "perlin.hpp"

Perlin::Perlin(unsigned gridSizeX, unsigned gridSizeY, unsigned seed):
 	_seed{seed}, _generator{seed}, _gridSizeX{gridSizeX}, _gridSizeY{gridSizeY} {
		initGradients();
}

float Perlin::val(float x, float y) {
	assert(x >= 0);
	assert(y >= 0);
	assert(x <= _gridSizeX-1);
	assert(y <= _gridSizeY-1);
	int x0 = std::floor(x);
	int x1 = x0 + 1;
	int y0 = std::floor(y);
	int y1 = y0 + 1;

	float sx = x - (float)x0;
	float sy = y - (float)y0;

	float n0, n1, ix0, ix1, value;
	n0 = dotGridGradient(x0, y0, x, y);
	n1 = dotGridGradient(x1, y0, x, y);
	ix0 = lerp(n0, n1, sx);
	n0 = dotGridGradient(x0, y1, x, y);
	n1 = dotGridGradient(x1, y1, x, y);
	ix1 = lerp(n0, n1, sx);
	value = lerp(ix0, ix1, sy);

	assert(value >= -1);
	assert(value <= 1);
	return value;
}

void Perlin::setSeed(unsigned seed) {
	_seed = seed;
	_generator.seed(seed);
	initGradients();
}

unsigned Perlin::getSeed() {
	return _seed;
}

Perlin::RandGrad::RandGrad(std::mt19937& generator) {
	static std::uniform_real_distribution<> dis(-1, 1);

	x = dis(generator);
	y = dis(generator);
	float gs = std::sqrt(x*x + y*y);
	x /= gs;
	y /= gs;
}

float Perlin::lerp(float a0, float a1, float w) {
	return (1.0 - w)*a0 + w*a1;
}

float Perlin::dotGridGradient(int ix, int iy, float x, float y) {
	float dx = x - (float)ix;
	float dy = y - (float)iy;

	return (dx*_gradients[iy][ix].x + dy*_gradients[iy][ix].y);
}

void Perlin::initGradients() {
	_gradients.resize(_gridSizeY, std::vector<RandGrad>(_gridSizeX, RandGrad(_generator)));
	for(auto& row : _gradients)
		for(auto& v : row)
			v = RandGrad(_generator);
}
