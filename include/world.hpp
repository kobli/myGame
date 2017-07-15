#ifndef WORLD_HPP_16_11_18_17_15_20
#define WORLD_HPP_16_11_18_17_15_20 
#include <vector>
#include <list>
#include <memory>
#include "main.hpp"
#include "controller.hpp"
#include "serializable.hpp"
#include "observableEntityComponent.hpp"
#include "keyValueStore.hpp"

using ec::ID;
using ec::NULLID;
enum ObjStaticID: ID {
	FIRST,
	Map = 10,
	Camera = 20,
	FIRSTFREE = 30
};
static_assert(ObjStaticID::FIRST != NULLID, "");

enum ComponentType: u8
{
	NONE = 0, 
	Body,
	GraphicsSphere,
	GraphicsMesh,
	GraphicsParticleSystem,
	Collision,
	Wizard,
	AttributeStore,
	LAST
};

typedef ec::EntityEvent<ComponentType> EntityEvent;
namespace std
{
	template <>
		struct hash<EntityEvent>
		{
			size_t operator()(EntityEvent const & e) const noexcept
			{
				return e.created + e.destroyed*2 + e.componentT*4 + ComponentType::LAST*4*e.entityID;
			}
		};
}

class ObservableComponentBase : public Observable<EntityEvent>, public Serializable {
	public:
		ObservableComponentBase(ID parentEntID, ComponentType realCompType);
		void notifyObservers();
	private:
		EntityEvent _updMsg;
};

typedef ec::ObservableEntity<ObservableComponentBase,ComponentType,EntityEvent>
	Entity;
typedef ec::ObservableEntityManager<ObservableComponentBase,ComponentType,Entity,EntityEvent>
	EntityManager;

////////////////////////////////////////////////////////////

class BodyComponent: public ObservableComponentBase
{
	public:
		BodyComponent(ID parentEntID, vec3f position = vec3f(0), quaternion rotation = quaternion(0,0,0,0), vec3f velocity = vec3f(0));
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
};

////////////////////////////////////////////////////////////

class GraphicsComponent: public ObservableComponentBase
{
	public:
		GraphicsComponent(ID parentEntID, ComponentType t, vec3f posOffset = vec3f(0), vec3f rotOffset = vec3f(0), vec3f scale = vec3f(1));
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
		SphereGraphicsComponent(ID parentEntID, float radius = 0, vec3f posOffset = vec3f(0), vec3f rotOffset = vec3f(0), vec3f scale = vec3f(1));
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
		MeshGraphicsComponent(ID parentEntID, string fileName = "", bool animated = false, vec3f posOffset = vec3f(0), vec3f rotOffset = vec3f(0), vec3f scale = vec3f(1));
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

class ParticleSystemGraphicsComponent: public GraphicsComponent
{
	public:
		ParticleSystemGraphicsComponent(ID parentEntID, ID effectID = NULLID, vec3f posOffset = vec3f(0), vec3f rotOffset = vec3f(0), vec3f scale = vec3f(1));
		virtual void serDes(SerDesBase& s);
		template <typename T>
			void doSerDes(T& t)
			{
				t & _effectID;
			}
		ID getEffectID();

	private:
		ID _effectID;
};

////////////////////////////////////////////////////////////

class CollisionComponent: public ObservableComponentBase
{
	public:
		CollisionComponent(ID parentEntID, float radius = 1, float height = 0, vec3f posOffset = vec3f(0,0,0), bool kinematic = false);
		float getRadius() const;
		void setRadius(float);
		float getHeight() const;
		void setHeight(float);
		vec3f getPosOffset() const;
		void setPosOffset(vec3f);
		bool isKinematic();

		virtual void serDes(SerDesBase& s);
		template <typename T>
			void doSerDes(T& t)
			{
				t & _radius;
				t & _height;
				t & _posOff;
				t & _kinematic;
			}

	private:
		float _radius;
		float _height;
		vec3f _posOff;
		bool _kinematic;
};

////////////////////////////////////////////////////////////

class WizardComponent: public ObservableComponentBase
{
	public:
		WizardComponent(ID parentEntID);
		virtual void serDes(SerDesBase& s);
		template <typename T>
			void doSerDes(T&)
			{
			}
};

////////////////////////////////////////////////////////////
class AttributeAffector {
	public:
		enum ModifierType: u8 {
			Add = 0,
			Mul = 1,
		};

		AttributeAffector(std::string attribute, ModifierType modifierType
				, float modifierValue, bool permanent, float period = 0);

		std::string getAffectedAttribute();
		ModifierType getModifierType();
		float getModifierValue();
		bool isPermanent();
		float getPeriod();

	private:
		std::string _attribute;
		ModifierType _modifierType;
		float _modifierValue;
		bool _permanent;
	 	float _period;
};

class AttributeStoreComponent: public ObservableComponentBase, KeyValueStore
{
	public:
		AttributeStoreComponent(ID parentEntID);

		void addAttribute(std::string key, float value);
		bool hasAttribute(std::string key);
		float getAttribute(std::string key);
		void setAttribute(std::string key, float value);
		void setOrAddAttribute(std::string key, float value);

		ID addAttributeAffector(AttributeAffector aa);
		bool removeAttributeAffector(ID affectorID);
		float getAttributeAffected(std::string key);

		virtual void serDes(SerDesBase& s);
		template <typename T>
			void doSerDes(T& t)
			{
			}

	private:
		SolidVector<AttributeAffector, ID, NULLID> _attributeAffectors;
};

////////////////////////////////////////////////////////////

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

class World: public Observabler<EntityEvent>
{
	public:
		World(WorldMap& wm);
		ID createEntity(ID hintEntID = NULLID);
		Entity& createAndGetEntity(ID hintEntID = NULLID);
		void removeEntity(ID entID);
		Entity* getEntity(ID entID);
		ID createCharacter(vec3f position);
		WorldMap& getMap();
		IterateOnly<SolidVector<Entity,ID,NULLID>> getEntities();

	private:
		WorldMap& _map;
		EntityManager _entManager;
};

template <typename T>
T& operator <<(T& t, const ObservableComponentBase& m)
{
	m >> t;
	return t;
}
template <typename T>
T& operator >>(T& t, ObservableComponentBase& m)
{
	m << t;
	return t;
}
#endif /* WORLD_HPP_16_11_18_17_15_20 */
