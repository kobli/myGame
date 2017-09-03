#ifndef TERRAINTEXTURER_HPP_17_08_25_21_30_11
#define TERRAINTEXTURER_HPP_17_08_25_21_30_11 
#include "main.hpp"

enum TerrainTexture{
	sand,
	grass,
	rock,
	snow,
};

class TerrainTexturer {
	public:
		static irr::video::SColor texture(f32 x, f32 y, f32 z, irr::core::vector3df /*normal*/)
		{
			typedef std::pair<TerrainTexture, Interval<float>> TextureLevel;
			static std::vector<TextureLevel> textureLevels{
				{TerrainTexture::sand, 	{0   , 0.25}},
					{TerrainTexture::grass, {0.25, 0.5}},
					{TerrainTexture::rock, 	{0.5 , 0.75}},
					{TerrainTexture::snow,  {0.75, 1}},
			};
			std::sort(textureLevels.begin(), textureLevels.end(), [&](const TextureLevel& a, const TextureLevel& b)->bool {
					return a.second.distanceFrom(z) < b.second.distanceFrom(z);
					});

			float d1 = textureLevels[0].second.distanceFrom(z);
			float d2 = textureLevels[1].second.distanceFrom(z);
			float dd = std::abs(d1-d2);

			static std::random_device rd;
			static std::mt19937 gen(rd());
			std::uniform_real_distribution<> dis(d1, (d2-d1)/dd+d1-0.5);
			float blend = dis(gen);
			static const float ps = 5;
			static Perlin p(ps+1,ps+1);
			blend *= (1.5+p.val(x*ps, y*ps))/3;
			
			//return SColor(255, textureLevels[0].first, textureLevels[1].first, (1-blend)*255);
			return SColor(255, textureLevels[0].first, textureLevels[1].first, (1-std::max(0.f, std::min(1.f,2*p.val(x*ps, y*ps))))*255);
		}

	private:
		// returns 1 for horizontal normal, 0 for vertical normal
		static inline float slope(vector3df normal) 
		{
			normal.normalize();
			return 1-vector3df(0,1,0).dotProduct(normal);
		}
};
#endif /* TERRAINTEXTURER_HPP_17_08_25_21_30_11 */
