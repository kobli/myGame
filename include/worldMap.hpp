#ifndef MAP_HPP_17_08_25_18_39_37
#define MAP_HPP_17_08_25_18_39_37 
#include <memory>
#include "terrain.hpp"
#include "treePlanter.hpp"

class WorldMap {
	public:
		WorldMap(): _terrain(nullptr)
		{}

		WorldMap(vec2u size, unsigned seed = 1): _terrain(nullptr)
		{
			generate(size, seed);
		}

		void generate(vec2u size, unsigned seed = 1)
		{
			_terrain.reset(new Terrain(size, seed));
			_trees = TreePlanter::plant(*_terrain, seed);
		}

		const Terrain& getTerrain() const
		{
			assert(_terrain.get() != nullptr);
			return *_terrain;
		}

		const std::vector<Tree>& getTrees() const
		{
			return _trees;
		}

		float getHeightAt(unsigned x, unsigned y) const
		{
			assert(_terrain.get() != nullptr);
			return _terrain->heightAt(x,y);
		}

		vec2u getSize() const 
		{
			assert(_terrain.get() != nullptr);
			return _terrain->size();
		}

	private:
		std::unique_ptr<Terrain> _terrain;
		std::vector<Tree> _trees;
};
#endif /* MAP_HPP_17_08_25_18_39_37 */
