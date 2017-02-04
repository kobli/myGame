#include "observer.hpp"
#include "world.hpp"
#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <map>

#ifndef SYSTEM_HPP_17_01_29_09_08_12
#define SYSTEM_HPP_17_01_29_09_08_12 

class System: public Observer<EntityEvent>
{
	public:
		inline virtual void update(float /*timeDelta*/) {}
};

class Physics: public System
{
	public:
		Physics(World& world, scene::ISceneManager* smgr = nullptr);
		vec3f getObjVelocity(u32 ID);
		virtual void update(float timeDelta);
		virtual void onObservableAdd(EntityEvent& m);
		virtual void onObservableUpdate(EntityEvent& m);
		virtual void onObservableRemove(EntityEvent& m);

	private:
		World& _world;
		unique_ptr<btDiscreteDynamicsWorld> _physicsWorld;
		struct ObjData {
			float walkTimer;
		};
		std::map<u32, ObjData> _objData;

		btCollisionObject* getCollisionObjectByID(int entityID);
		void bodyDoStrafe(float timeDelta);
};

class ViewSystem: public System
{
	public:
		ViewSystem(irr::scene::ISceneManager* smgr, World& world);
		virtual void onObservableAdd(EntityEvent& m);
		virtual void onObservableUpdate(EntityEvent& m);
		virtual void onObservableRemove(EntityEvent& m);

	private:
		irr::scene::ISceneManager* _smgr;
		World& _world;
		//irr::scene::IMetaTriangleSelector* _worldTS;

};
#endif /* SYSTEM_HPP_17_01_29_09_08_12 */
