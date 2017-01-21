#include <observer.hpp>
#include <world.hpp>
#ifndef SCENENODEMANAGER_HPP_17_01_21_09_41_54
#define SCENENODEMANAGER_HPP_17_01_21_09_41_54 

// creates and destroys scene nodes based on world events 
class SceneNodeManager: Observer<EntityEvent>
{
	public:
		SceneNodeManager(irr::scene::ISceneManager* smgr, World& world);
		virtual void onObservableAdd(EntityEvent& m);
		virtual void onObservableUpdate(EntityEvent& m);
		virtual void onObservableRemove(EntityEvent& m);

	private:
		irr::scene::ISceneManager* _smgr;
		World& _world;
};
#endif /* SCENENODEMANAGER_HPP_17_01_21_09_41_54 */
