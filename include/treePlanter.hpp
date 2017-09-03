#ifndef TREEPLANTER_HPP_17_08_18_19_15_15
#define TREEPLANTER_HPP_17_08_18_19_15_15 
#include <vector>
#include "main.hpp"
#include "terrain.hpp"
#include "perlinMultiOctave.hpp"

struct Tree {
	irr::core::vector3df position;
};

class TreePlanter {
		TreePlanter();
	public:
		static inline std::vector<Tree> plant(const Terrain& t, unsigned seed = 1) {
			PerlinMultiOctave forestMap(t.size().X, t.size().Y, PerlinMultiOctave::Octaves(0.2, 0.5, 2, 0.1), seed);
			SAVEIMAGE("forestmap.bmp", [&](unsigned x, unsigned y){ return (std::min(1.f,std::max(0.f,forestMap.val(x,y)))>0.5)*255; }, t.size());

			std::vector<Tree> r;

			const int avgTreeSpace = 10;
			for(unsigned y = 0; y < t.size().Y; y += avgTreeSpace)
				for(unsigned x = 0; x < t.size().X; x += avgTreeSpace)
					if(forestMap.val(x,y) > 0.5)
						r.push_back(Tree{irr::core::vector3df(x, t.heightAt(x,y), y)});
			return r;
		}
};
#endif /* TREEPLANTER_HPP_17_08_18_19_15_15 */
