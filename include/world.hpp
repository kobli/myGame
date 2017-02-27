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
	GraphicsMesh,
	Collision,
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
		WorldMap(float patchSize, scene::ISceneManager* scene);
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
		BodyComponent(WorldEntity& parent, vec3f position = vec3f(0), quaternion rotation = quaternion(0,0,0,0), vec3f velocity = vec3f(0));
		vec3f getPosition() const;
		quaternion getRotation() const;
		vec3f getVelocity() const;
		i8 getRotDir() const;
		void setPosition(vec3f);
		void setRotation(quaternion);
		void setVelocity(vec3f);
		void setStrafeDir(vec2f strafeDir);
		void setRotDir(i8 rotDir);
		void update(float timeDelta);
		bool posOrRotChanged();
		//vec3f getTotalVelocity() const;
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
		quaternion _rotation; // radians
		vec3f _velocity; // speed and direction at which object is moving
		// strafe velocity is relative to rotation and will be added to normal velocity
		// (unless the entity has a collision component and the entity is falling)
		vec2f _strafeDir; 
		float _strafeSpeed;
		i8 _rotDir; // around Y axis: left = -1, stop = 0, right = 1
		bool _posRotChanged; // is set during notifyObservers call when the call is caused by change of position or rotation
};

////////////////////////////////////////////////////////////

class GraphicsComponent: public WorldEntityComponent
{
	public:
		GraphicsComponent(ComponentType t, WorldEntity& parent, vec3f posOffset = vec3f(0), vec3f rotOffset = vec3f(0), vec3f scale = vec3f(1));
		vec3f getPosOffset();
		vec3f getRotOffset();
		vec3f getScale();
		void setPosOffset(vec3f);
		void setRotOffset(vec3f);
		void setScale(vec3f);
		virtual void serDes(SerDesBase& s);
		template <typename T>
			void doSerDes(T& t)
			{
				t & _posOff;
				t & _rotOff;
				t & _scale;
			}

	private:
		vec3f _posOff;
		vec3f _rotOff; // degrees
		vec3f _scale;
};

class SphereGraphicsComponent: public GraphicsComponent 
{
	public:
		SphereGraphicsComponent(WorldEntity& parent, float radius = 0, vec3f posOffset = vec3f(0), vec3f rotOffset = vec3f(0), vec3f scale = vec3f(1));
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

class MeshGraphicsComponent: public GraphicsComponent
{
	public:
		MeshGraphicsComponent(WorldEntity& parent, string fileName = "", bool animated = false, vec3f posOffset = vec3f(0), vec3f rotOffset = vec3f(0), vec3f scale = vec3f(1));
		virtual void serDes(SerDesBase& s);
		template <typename T>
			void doSerDes(T& t)
			{
				t & _fileName;
				t & _animated;
			}
		string getFileName();
		bool isAnimated();

	private:
		string _fileName;
		bool _animated;
};

////////////////////////////////////////////////////////////

class CollisionComponent: public WorldEntityComponent
{
	public:
		CollisionComponent(WorldEntity& parent, float radius = 1, float height = 0, vec3f posOffset = vec3f(0,0,0));
		float getRadius() const;
		void setRadius(float);
		float getHeight() const;
		void setHeight(float);
		vec3f getPosOffset() const;
		void setPosOffset(vec3f);

		virtual void serDes(SerDesBase& s);
		template <typename T>
			void doSerDes(T& t)
			{
				t & _radius;
				t & _height;
				t & _posOff;
			}

	private:
		float _radius;
		float _height;
		vec3f _posOff;
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
		void collisionCallback(u32 objID, u32 otherObjID);
};

////////////////////////////////////////////////////////////

class WorldEntity: public Observabler<EntityEvent>
{
	public:
		WorldEntity(World& w, u32 ID);
		u32 getID();
		shared_ptr<BodyComponent> setBodyComponent(shared_ptr<BodyComponent> bc);
		shared_ptr<BodyComponent> getBodyComponent();
		shared_ptr<GraphicsComponent> setGraphicsComponent(shared_ptr<GraphicsComponent> gc);
		shared_ptr<GraphicsComponent> getGraphicsComponent();
		shared_ptr<CollisionComponent> setCollisionComponent(shared_ptr<CollisionComponent> cc);
		shared_ptr<CollisionComponent> getCollisionComponent();
		shared_ptr<WizardComponent> setWizardComponent(shared_ptr<WizardComponent> cc);
		shared_ptr<WizardComponent> getWizardComponent();

		World& _world;

	private:
		const u32 _ID;
		shared_ptr<BodyComponent> _body;
		shared_ptr<GraphicsComponent> _graphics;
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
