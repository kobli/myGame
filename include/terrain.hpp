#ifndef TERRAIN_HPP_17_08_18_23_13_00
#define TERRAIN_HPP_17_08_18_23_13_00 
#include <vector>
#include <functional>
#include <cassert>
#include "main.hpp"

class Terrain {
	public:
	Terrain(vec2u size, unsigned seed = 1);
	vec2u size() const;
	float heightAt(float x, float y) const;
	vec3f normalAt(float x, float y) const;
	void setSeed(unsigned seed);
	unsigned getSeed();

	bool dumpHeightmapToImage(std::string fileName);

	private:
	class HeightMap
	{
		private:
			const vec2u _size;
			std::vector<float> data;
			static inline float lerp(float a, float b, float p) {
				assert(p >= 0 && p <= 1);
				return a*(1-p) + b*p;
			}
		public:
			// should return values in range -0.5, 0.5
			typedef std::function<float(unsigned x, unsigned y)> Generator;

			HeightMap(vec2u size) : _size{size}, data(0)
			{
				data.reserve(_size.X*_size.Y);
			}

			void generate(Generator f)
			{
				unsigned i=0;
				for(unsigned y = 0; y < _size.Y; ++y)
					for(unsigned x = 0; x < _size.X; ++x)
						set(i++, f(x, y));
			}

			vec2u size() const
			{
				return _size;
			}

			void set(unsigned x, unsigned y, float z) { data[y * _size.X + x] = z; }
			void set(unsigned i, float z) { data[i] = z; }
			float get(unsigned x, unsigned y) const {
				assert(x >= 0);
				assert(y >= 0);
				assert(x < _size.X);
				assert(y < _size.Y);
				return data[y * _size.X + x];
		 	}
			float get(float x, float y) const
			{
				assert(x >= 0);
				assert(y >= 0);
				unsigned xi = x;
				unsigned yi = y;
				if(x == xi && y == yi)
					return get(xi, yi);

				unsigned xi1 = std::min(_size.X-1, xi+1);
				unsigned yi1 = std::min(_size.Y-1, yi+1);

				float z1 = lerp(get(xi, yi), get(xi1, yi), x-xi);
				float z2 = lerp(get(xi, yi1), get(xi1, yi1), x-xi);
				return lerp(z1, z2, y-yi);
			}

			irr::core::vector3df getNormal(float x, float y) const
			{
				x = unsigned(x);
				y = unsigned(y);

				const float zc = get(x, y);
				float zl, zr, zu, zd;

				if (x < 1)
				{
					zr = get(x + 1, y);
					zl = zc + zc - zr;
				}
				else if (x == _size.X - 1)
				{
					zl = get(x - 1, y);
					zr = zc + zc - zl;
				}
				else
				{
					zr = get(x + 1, y);
					zl = get(x - 1, y);
				}

				if (y < 1)
				{
					zd = get(x, y + 1);
					zu = zc + zc - zd;
				}
				else if (y == _size.Y - 1)
				{
					zu = get(x, y - 1);
					zd = zc + zc - zu;
				}
				else
				{
					zd = get(x, y + 1);
					zu = get(x, y - 1);
				}
				return irr::core::vector3df(2 * (zl - zr), 4, 2 * (zd - zu)).normalize();
			}
	};

	vec2u _size;
	HeightMap _heightMap;
	unsigned _seed;

	void init();
};
#endif /* TERRAIN_HPP_17_08_18_23_13_00 */
