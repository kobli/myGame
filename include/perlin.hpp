#ifndef PERLIN_HPP_17_08_05_11_28_12
#define PERLIN_HPP_17_08_05_11_28_12 
#include <vector>
#include <random>

class Perlin {
	public:
		Perlin(unsigned gridSizeX, unsigned gridSizeY, unsigned seed = 1);
		float val(float x, float y);
		void setSeed(unsigned seed);
		unsigned getSeed();

	private:
		unsigned _seed;
		std::mt19937 _generator;

		struct RandGrad {
			RandGrad(std::mt19937& generator);
			float x;
			float y;
		};

		unsigned _gridSizeX;
		unsigned _gridSizeY;

		std::vector<std::vector<RandGrad>> _gradients;
		float lerp(float a0, float a1, float w);
		float dotGridGradient(int ix, int iy, float x, float y);
		void initGradients();
};

#endif /* PERLIN_HPP_17_08_05_11_28_12 */
