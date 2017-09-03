#ifndef TERRAIN_HPP_17_08_18_23_13_00
#define TERRAIN_HPP_17_08_18_23_13_00 
#include <vector>
#include <functional>
#include "main.hpp"

class Terrain {
	public:
	Terrain(vec2u size, unsigned seed = 1);
	vec2u size() const;
	float heightAt(unsigned x, unsigned y) const;
	vec3f normalAt(unsigned x, unsigned y) const;
	void setSeed(unsigned seed);
	unsigned getSeed();

	bool dumpHeightmapToImage(std::string fileName);

	private:
	class HeightMap
	{
		private:
			const vec2u _size;
			std::vector<float> data;
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
			float get(unsigned x, unsigned y) const { return data[y * _size.X + x]; }

			irr::core::vector3df getNormal(unsigned x, unsigned y) const
			{
				const float zc = get(x, y);
				float zl, zr, zu, zd;

				if (x == 0)
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

				if (y == 0)
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
