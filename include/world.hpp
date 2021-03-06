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
#include "worldMap.hpp"
#include "ringBuffer.hpp"

using ec::ID;
using ec::NULLID;
enum ObjStaticID: ID {
	NULLOBJ = 0,
	FIRSTFREE = 1,
	Camera = 		1<<12,
	Map = 			1<<13,
	Skybox = 		1<<14,
	OBJCHILD = 	1<<15,
};
static_assert(OBJCHILD == u64(1<<15));

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
		BodyComponent(ID parentEntID, vec3f position = vec3f(0), quaternion rotation = quaternion(0,0,0,1), vec3f velocity = vec3f(0));
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
		CollisionComponent(ID parentEntID, float radius = 1, float height = 0, vec3f posOffset = vec3f(0,0,0), float mass = 0, bool kinematic = false, float gravity = -10);
		float getRadius() const;
		void setRadius(float);
		float getHeight() const;
		void setHeight(float);
		vec3f getPosOffset() const;
		void setPosOffset(vec3f);
		bool isKinematic();
		float getMass();
		float getGravity();
		void setSlippery(bool slippery);
		bool isSlippery();

		virtual void serDes(SerDesBase& s);
		template <typename T>
			void doSerDes(T& t)
			{
				t & _radius;
				t & _height;
				t & _posOff;
				t & _kinematic;
				t & _mass;
				t & _gravity;
				t & _slippery;
			}

	private:
		float _radius;
		float _height;
		vec3f _posOff;
		float _mass;
		bool _kinematic;
		float _gravity;
		bool _slippery;
};

////////////////////////////////////////////////////////////

class WizardComponent: public ObservableComponentBase
{
	public:
		WizardComponent(ID parentEntID);
		virtual void serDes(SerDesBase& s);
		template <typename T>
			void doSerDes(T& t)
			{
				t & _currentJob;
				t & _currentJobEffectId;
				t & _currentJobDuration;
				t & _currentJobProgress;
				t & _spellInHandsPower;
				t & _spellInHandsRadius;
				t & _spellInHandsSpeed;
				t & _availableBodyC;
				t & _totalBodyC;
				t & _spellInHandsEffects;
				t & _commandQueue;
			}

		void setCurrentJobStatus(std::string job, int jobEffectId, float duration, float progress);
		void setSpellInHandsData(float power, float radius, float speed, std::vector<unsigned> effects);
		void setBodyStatus(unsigned available, unsigned total);
		void setCommandQueue(std::vector<unsigned> commands);
		std::string getCurrentJob();
		int getCurrentJobEffectId();
		float getCurrentJobDuration();
		float getCurrentJobProgress();
		bool hasSpellInHands();
		float getSpellInHandsPower();
		float getSpellInHandsRadius();
		float getSpellInHandsSpeed();
		const std::vector<unsigned>& getSpellInHandsEffects();
		unsigned getAvailableBodyC();
		unsigned getTotalBodyC();
		const std::vector<unsigned>& getCommandQueue();

	private:
		std::string _currentJob;
		int _currentJobEffectId;
		float _currentJobDuration;
		float _currentJobProgress;
		float _spellInHandsPower;
		float _spellInHandsRadius;
		float _spellInHandsSpeed;
		std::vector<unsigned> _spellInHandsEffects;
		std::vector<unsigned> _commandQueue;
		unsigned _availableBodyC;
		unsigned _totalBodyC;
};

////////////////////////////////////////////////////////////
class AttributeAffector {
	public:
		enum ModifierType: u8 {
			Add = 0,
			Mul = 1,
		};

		AttributeAffector(ID author, std::string attribute, ModifierType modifierType
				, float modifierValue, bool permanent);

		std::string getAffectedAttribute();
		ModifierType getModifierType();
		float getModifierValue();
		bool isPermanent();
		ID getAuthor();

	private:
		std::string _attribute;
		ModifierType _modifierType;
		float _modifierValue;
		bool _permanent;
		ID _author;
};

class AttributeStoreComponent: public ObservableComponentBase, KeyValueStore
{
	public:
		AttributeStoreComponent(ID parentEntID);

		void addAttribute(std::string key, float value);
		void addAttribute(std::string key, std::string value);
		bool hasAttribute(std::string key);
		template <typename T>
		T getAttribute(std::string key) const
		{
			return getValue<T>(key);
		}
		void setAttribute(std::string key, float value);
		void setAttribute(std::string key, std::string value);
		void setOrAddAttribute(std::string key, float value);
		void setOrAddAttribute(std::string key, std::string value);

		ID addAttributeAffector(AttributeAffector aa);
		bool removeAttributeAffector(ID affectorID);
		float getAttributeAffected(std::string key);
		std::vector<AttributeAffector> getAttributeAffectorHistory();

		virtual void serDes(SerDesBase& s);
		template <typename T>
			void doSerDes(T& t)
			{
			}

	private:
		SolidVector<AttributeAffector, ID, NULLID> _attributeAffectors;
		RingBuffer<AttributeAffector> _attributeAffectorHistory;
};

////////////////////////////////////////////////////////////

class World: public Observabler<EntityEvent>
{
	public:
		World(const WorldMap& wm);
		ID createEntity(ID hintEntID = NULLID);
		Entity& createAndGetEntity(ID hintEntID = NULLID);
		void removeEntity(ID entID);
		Entity* getEntity(ID entID);
		ID createCharacter(vec3f position);
		const WorldMap& getMap();
		IterateOnly<SolidVector<Entity,ID,NULLID>> getEntities();

	private:
		const WorldMap& _map;
		EntityManager _entManager;
};

////////////////////////////////////////////////////////////

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
