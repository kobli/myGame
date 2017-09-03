#ifndef MAP_HPP_17_08_25_18_39_37
#define MAP_HPP_17_08_25_18_39_37 
#include <memory>
#include "terrain.hpp"
#include "treePlanter.hpp"

class Map {
	public:
		Map(): _terrain(nullptr)
		{}

		void generate(vec2u size, unsigned seed = 1)
		{
			_terrain.reset(new Terrain(size, seed));
			_trees = TreePlanter::plant(*_terrain, seed);
		}

		const Terrain& getTerrain()
		{
			assert(_terrain.get() != nullptr);
			return *_terrain;
		}

		const std::vector<Tree>& getTrees() 
		{
			return _trees;
		}

	private:
		std::unique_ptr<Terrain> _terrain;
		std::vector<Tree> _trees;
};
#endif /* MAP_HPP_17_08_25_18_39_37 */
