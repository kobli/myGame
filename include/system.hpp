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
		System(World& world);
		inline virtual void update(float /*timeDelta*/) {}
		inline virtual void onObservableAdd(EntityEvent&) {}
		inline virtual void onObservableUpdate(EntityEvent&) {}
		inline virtual void onObservableRemove(EntityEvent&) {}
	protected:
		World& _world;
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
		void registerCollisionCallback(std::function<void(u32, u32)> callback);
		void registerPairCollisionCallback(std::function<void(u32, u32)> callback);

	private:
		unique_ptr<btDiscreteDynamicsWorld> _physicsWorld;
		struct ObjData {
			float walkTimer = 0;
			bool onGround = false;
		};
		std::map<u32, ObjData> _objData;
		std::vector<std::function<void(u32, u32)>> _collCallbacks;
		float _tAcc;
		bool _updating;

		btCollisionObject* getCollisionObjectByID(int entityID);
		void bodyDoStrafe(float timeDelta);
		void moveKinematics(float timeDelta);
		void callCollisionCBs();
};

class ViewSystem: public System
{
	public:
		ViewSystem(irr::scene::ISceneManager* smgr, World& world);
		virtual void onObservableAdd(EntityEvent& m);
		virtual void onObservableUpdate(EntityEvent& m);
		virtual void onObservableRemove(EntityEvent& m);
		virtual void update(float timeDelta);

	private:
		irr::scene::ISceneManager* _smgr;
		std::list<u32> _transformedEntities;
		//irr::scene::IMetaTriangleSelector* _worldTS;
		
		void updateTransforms(float timeDelta);
};

class SpellSystem: public System
{
	public:
		SpellSystem(World& world);
		~SpellSystem();
		virtual void update(float timeDelta);
		virtual void onObservableUpdate(EntityEvent& m);
		void reload();
		void addWizard(u32 ID);
		void removeWizard(u32 ID);
		void cast(std::string& incantation, u32 authorID);
		void collisionCallback(u32 objID, u32 otherObjID);

	private:
		lua_State* _luaState;

		void init();
		void deinit();
		u32 launchSpell(float radius, float speed, u32 wizard);
		void removeSpell(u32 spell);
};

class InputSystem: public System
{
	public:
		InputSystem(World& world, SpellSystem& spells);
		void handleCommand(Command& c, u32 controlledObjID);

	private:
		BodyComponent* getBodyComponent(u32 objID);
		SpellSystem& _spells;
};

#endif /* SYSTEM_HPP_17_01_29_09_08_12 */
