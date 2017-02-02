#include <main.hpp>
#include <vector>
#include <list>
#include <memory>
#include <controller.hpp>
#include <observer.hpp>
#include <serializable.hpp>

#ifndef WORLD_HPP_16_11_18_17_15_20
#define WORLD_HPP_16_11_18_17_15_20 

enum ComponentType: u8
{
	None, 
	Body,
	GraphicsSphere,
	Collision,
	Input,
	Wizard,
};

class WorldEntityComponent;
class EntityEvent
{
	public:
		EntityEvent(u32 entityID = 0, ComponentType componentModifiedType = ComponentType::None
				, WorldEntityComponent* componentModified = nullptr, bool created = false, bool destroyed = false);
		u32 _entityID;
		ComponentType _componentModifiedType;
		WorldEntityComponent* _componentModified;
		bool _created;
		bool _destroyed;
};

class World;
class WorldEntity;

class WorldMap
{
	public:
		WorldMap(unsigned patchSize, scene::ISceneManager* scene);
		float* getHeightMap();
		unsigned getVertexCount();
		float getPatchSize();
		float getHeightScale();

	private:
		const float _patchSize;
		scene::ISceneManager* _scene;
		std::unique_ptr<float[]> _heightMap;
		float _heightScale;
		unsigned _vertexC;
};

////////////////////////////////////////////////////////////

class WorldEntityComponent: public Observable<EntityEvent>, public Serializable
{
	public:
		WorldEntityComponent(WorldEntity& parent, ComponentType _compType);
		void notifyObservers();

	protected:
		WorldEntity& _parent;
		const ComponentType _compType;
};

////////////////////////////////////////////////////////////

class BodyComponent: public WorldEntityComponent
{
	public:
		BodyComponent(WorldEntity& parent, vec3f position = vec3f(0), vec3f rotation = vec3f(0,0,0), vec3f velocity = vec3f(0));
		vec3f getPosition() const;
		vec3f getRotation() const;
		vec3f getVelocity() const;
		i32 getRotDir() const;
		void setPosition(vec3f);
		void setRotation(vec3f);
		void setVelocity(vec3f);
		void setStrafeDir(vec2f strafeDir);
		void setRotDir(i32 rotDir);
		void update(float timeDelta);
		bool posOrRotChanged();
		vec3f getTotalVelocity() const;
		vec2f getStrafeDir() const;
		float getStrafeSpeed() const;
		virtual void serDes(SerDesBase& s);
		template <typename T>
			void doSerDes(T& t)
			{
				t & _strafeDir;
				t & _rotDir;
				t & _position;
				t & _rotation;
				t & _velocity;
			}

	private:
		vec3f _position;
		vec3f _rotation; // degrees
		vec3f _velocity; // speed and direction at which object is moving
		// strafe velocity is relative to rotation and will be added to normal velocity
		// (unless the entity has a collision component and the entity is falling)
		vec2f _strafeDir; 
		float _strafeSpeed;
		i32 _rotDir; // around Y axis: left = -1, stop = 0, right = 1
		bool _posRotChanged; // is set during notifyObservers call when the call is caused by change of position or rotation
};

////////////////////////////////////////////////////////////

class GraphicsComponent: public WorldEntityComponent
{
	public:
		GraphicsComponent(ComponentType t, WorldEntity& parent, vec3f posOffset = vec3f(0), vec3f rotOffset = vec3f(0));
		vec3f getPosOffset();
		vec3f getRotOffset();
		void setPosOffset(vec3f);
		void setRotOffset(vec3f);
		virtual void serDes(SerDesBase& s);
		template <typename T>
			void doSerDes(T& t)
			{
				t & _posOff;
				t & _rotOff;
			}


	private:
		vec3f _posOff;
		vec3f _rotOff; // degrees
};

class SphereGraphicsComponent: public GraphicsComponent 
{
	public:
		SphereGraphicsComponent(WorldEntity& parent, float radius = 0, vec3f posOffset = vec3f(0), vec3f rotOffset = vec3f(0));
		virtual void serDes(SerDesBase& s);
		template <typename T>
			void doSerDes(T& t)
			{
				t & _radius;
			}
		float getRadius();
		void setRadius(float radius);

	private:
		float _radius;
};

////////////////////////////////////////////////////////////

class CollisionComponent: public WorldEntityComponent
{
	public:
		CollisionComponent(WorldEntity& parent, vec3f ellipsoidRadius = vec3f(0,0,0));
		//vec3f getCollisionResultPosition(float timeDelta);
		vec3f getColliderRadius() const;
		void setColliderRadius(vec3f);

		virtual void serDes(SerDesBase& s);
		template <typename T>
			void doSerDes(T& t)
			{
				t & _ellipsoidRadius;
			}


		vec3f _ellipsoidRadius;
};

////////////////////////////////////////////////////////////

class InputComponent: public WorldEntityComponent
{
	public:
		InputComponent(WorldEntity& parent);
		void handleCommand(Command& c);
		virtual void serDes(SerDesBase& s);
		template <typename T>
			void doSerDes(T&)
			{
			}

};

////////////////////////////////////////////////////////////

class WizardComponent: public WorldEntityComponent
{
	public:
		WizardComponent(WorldEntity& parent);
		~WizardComponent();
		void cast(std::string& incantation);
		static void update(float timeDelta);
		virtual void serDes(SerDesBase& s);
		template <typename T>
			void doSerDes(T&)
			{
			}


	private:
		static std::shared_ptr<lua_State> _luaState;
		u32 launchSpell(float radius, float speed);
};

////////////////////////////////////////////////////////////

class WorldEntity: public Observabler<EntityEvent>
{
	public:
		WorldEntity(World& w, u32 ID);
		u32 getID();
		bool isEmpty(); // true when entity does not contain any components
		shared_ptr<BodyComponent> setBodyComponent(shared_ptr<BodyComponent> bc);
		shared_ptr<BodyComponent> getBodyComponent();
		shared_ptr<GraphicsComponent> setGraphicsComponent(shared_ptr<GraphicsComponent> gc);
		shared_ptr<GraphicsComponent> getGraphicsComponent();
		shared_ptr<InputComponent> setInputComponent(shared_ptr<InputComponent> ic);
		shared_ptr<InputComponent> getInputComponent();
		shared_ptr<CollisionComponent> setCollisionComponent(shared_ptr<CollisionComponent> cc);
		shared_ptr<CollisionComponent> getCollisionComponent();
		shared_ptr<WizardComponent> setWizardComponent(shared_ptr<WizardComponent> cc);
		shared_ptr<WizardComponent> getWizardComponent();

		World& _world;

	private:
		const u32 _ID;
		shared_ptr<BodyComponent> _body;
		shared_ptr<GraphicsComponent> _graphics;
		shared_ptr<InputComponent> _input;
		shared_ptr<CollisionComponent> _collision;
		shared_ptr<WizardComponent> _wizard;
};

////////////////////////////////////////////////////////////

class World: public Observabler<EntityEvent>
{
	public:
		World(WorldMap& wm);
		WorldEntity& createEntity(u32 ID = 0);
		void removeEntity(WorldEntity& e);
		WorldEntity& createCharacter(vec3f position);
		WorldMap& getMap();
		std::list<WorldEntity>& getEntities();
		//void update(float timeDelta);
		WorldEntity* getEntityByID(u32 ID);

	private:
		WorldMap& _map;
		u32 _nextEntityID;
		std::list<WorldEntity> _entities; // so that entityComponents can have references to entities
};

template <typename T>
T& operator <<(T& t, const WorldEntityComponent& m)
{
	m >> t;
	return t;
}
template <typename T>
T& operator >>(T& t, WorldEntityComponent& m)
{
	m << t;
	return t;
}
#endif /* WORLD_HPP_16_11_18_17_15_20 */
