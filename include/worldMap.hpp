#ifndef MAP_HPP_17_08_25_18_39_37
#define MAP_HPP_17_08_25_18_39_37 
#include <memory>
#include "terrain.hpp"
#include "treePlanter.hpp"
#include "serializable.hpp"

struct Spawnpoint {
	vec3f position;
};

class WorldMap: public Serializable {
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
			placeSpawnpoints();
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
		
		bool isInGround(vec3f p) const
		{
			assert(_terrain.get() != nullptr);
			if(!_terrain->contains(p.X, p.Z))
				return false;
			else
				return _terrain->heightAt(p.X, p.Z) > p.Y;
		}

		vec2u getSize() const 
		{
			assert(_terrain.get() != nullptr);
			return _terrain->size();
		}

		virtual void serDes(SerDesBase& s)
		{
			s.serDes(*this);
		}

		template <typename T>
			void doSer(T& t)
			{
				t & getSize();
				t & _terrain->getSeed();
			}

		template <typename T>
			void doDes(T& t)
			{
				vec2u size;
				unsigned seed;
				t & size;
				t & seed;
				generate(size, seed);
			}

		const std::vector<Spawnpoint>& getSpawnpoints() const
		{
			return _spawns;
		}

	private:
		std::unique_ptr<Terrain> _terrain;
		std::vector<Tree> _trees;
		std::vector<Spawnpoint> _spawns;

		void placeSpawnpoints()
		{
			unsigned edgePadding = 10;
			unsigned step = 5;
			float minTreeDistance = 2;
			unsigned first = edgePadding;
			unsigned last = int(getSize().X-1-edgePadding)/step*step;
			for(unsigned y = first; y <= last; y += step)
				for(unsigned x = first; x <= last; x += step)
				{
					if(x != first && x != last && y != first && y != last)
						continue;
					else {
						if(nearestTreeDistance(vec2u(x,y)) >= minTreeDistance)
							_spawns.push_back(Spawnpoint{vec3f(x,getHeightAt(x,y),y)});
					}
				}
		}

		float nearestTreeDistance(vec2u from)
		{
			float r = std::numeric_limits<float>::max();
			for(const Tree& t : _trees)
			{
				float d = vec2f(t.position.X, t.position.Z).getDistanceFrom(vec2f(from.X, from.Y));
				if(d < r)
					r = d;
			}
			return r;
		}
};

#endif /* MAP_HPP_17_08_25_18_39_37 */
