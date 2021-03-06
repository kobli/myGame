#ifndef SYSTEM_HPP_17_01_29_09_08_12
#define SYSTEM_HPP_17_01_29_09_08_12 
#include <map>
#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include "world.hpp"
#include "observer.hpp"

class System: public Observer<EntityEvent>
{
	public:
		System(World& world);
		inline virtual void update(float /*timeDelta*/) {}
		inline virtual void onMsg(const EntityEvent&) {}
	protected:
		World& _world;
};

class Physics: public System
{
	public:
		Physics(World& world, scene::ISceneManager* smgr = nullptr);
		vec3f getObjVelocity(ID objID);
		virtual void update(float timeDelta);
		virtual void onMsg(const EntityEvent& m);
		void registerCollisionCallback(std::function<void(ID, ID)> callback);
		void registerPairCollisionCallback(std::function<void(ID, ID)> callback);

	private:
		unique_ptr<btDiscreteDynamicsWorld> _physicsWorld;
		struct ObjData {
			float walkTimer = 0;
			bool onGround = false;
		};
		std::map<ID, ObjData> _objData;
		std::vector<std::function<void(ID, ID)>> _collCallbacks;
		float _tAcc;
		bool _updating;
		std::unique_ptr<float[]> _heightMap;

		btCollisionObject* getCollisionObjectByID(ID objID);
		void bodyDoStrafe(float timeDelta);
		void moveKinematics(float timeDelta);
		void callCollisionCBs();
};

class ViewSystem: public System
{
	public:
		ViewSystem(irr::scene::ISceneManager* smgr, World& world);
		~ViewSystem();
		virtual void onMsg(const EntityEvent& m);
		virtual void update(float timeDelta);

	private:
		irr::scene::ISceneManager* _smgr;
		std::set<ID> _transformedEntities;
		
		void spotObjects();
		void onObjectLookAt(scene::ISceneNode* sn);
		void removeAnimatorsAndTemporarilyShowChildByName(scene::ISceneNode* sn, std::string childName, u32 visibilityTimeoutMs);
		scene::ISceneNode* getOrCreateBaseSceneNode(ID entityID);
		bool isSceneNodeVisible(scene::IMeshSceneNode* sn);
		void updateTransforms(float timeDelta);
		scene::IParticleEmitter* addParticleEffect(ID effectID, scene::IParticleSystemSceneNode* sn);
		void loadTerrain();
};

class SpellSystem: public System
{
	public:
		SpellSystem(World& world);
		~SpellSystem();
		virtual void update(float timeDelta);
		virtual void onMsg(const EntityEvent& m);
		void reload();
		void addWizard(ID entID);
		void removeWizard(ID entID);
		void cast(std::string& incantation, ID author);
		void collisionCallback(ID objID, ID otherObjID);

	private:
		lua_State* _luaState;
		void lUpdate(float timeDelta);
		void lReportWalkingWizard(ID wizID, bool walking);

		void init();
		void deinit();
		ID launchSpell(float radius, float speed, float elevation, ID wizard, ID spellEffectID);
		void removeSpell(ID spell);
		ID addAttributeAffectorTo(ID eID, ID authorID, std::string attributeName
				, AttributeAffector::ModifierType modifierType
				, float modifierValue, bool permanent, float period);
};

class InputSystem: public System
{
	public:
		InputSystem(World& world, SpellSystem& spells);
		void handleCommand(Command& c, ID controlledObjID);

	private:
		BodyComponent* getBodyComponent(ID entID);
		SpellSystem& _spells;
};

#endif /* SYSTEM_HPP_17_01_29_09_08_12 */
