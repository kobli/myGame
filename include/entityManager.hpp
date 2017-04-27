#ifndef ENTITYMANAGER_HPP_17_04_20_11_26_38
#define ENTITYMANAGER_HPP_17_04_20_11_26_38 
#include <vector>
#include "solidVector.hpp"
#include "entity.hpp"
#include <memory>
#include <map>
#include <typeindex>
#include <typeinfo>

class ComponentContainerBase {
	public:
		virtual Component::ID emplace() = 0;
		virtual ComponentBase* get(Component::ID id) = 0;
		virtual void remove(Component::ID id) = 0;
};


template <typename T>
class ComponentContainer : public ComponentContainerBase {
	friend EntityManager;

	public:
		typedef typename SolidVector<T>::iterator iterator;

		iterator begin() {
			return _vec.begin();
		}

		iterator end() {
			return _vec.end();
		}

	protected:
		Component::ID emplace() {
			return _vec.emplace();
		}

		ComponentBase* get(Component::ID id) {
			if(_vec.indexValid(id))
				return &_vec[id];
			else
				return nullptr;
		}

		void remove(Component::ID id) {
			_vec.remove(id);
		}

	private:
		SolidVector<T> _vec;
};


class EntityManager {
	friend Entity;

	public:
		Entity::ID createEntity() {
			return _entities.insert(Entity(*this));
		}

		Entity* getEntity(Entity::ID id) {
			if(_entities.indexValid(id))
				return &_entities[id];
			else
				return nullptr;
		}

		void removeEntity(Entity::ID id) {
			if(_entities.indexValid(id))
				_entities.remove(id);
		}

		template <typename ComponentClass>
		void registerComponentType(ComponentType t) {
			if(!existsBucketFor(t)) {
				_componentBuckets[t] = std::make_unique<ComponentContainer<ComponentClass>>();
				_componentClassToType[typeid(ComponentClass)] = t;
			}
			else
				throw std::invalid_argument("Component class " + std::string(typeid(ComponentClass).name()) + " is already registered.");
		}

		template <typename ComponentClass>
		ComponentContainer<ComponentClass>& getComponentBucket() {
			ComponentType t = componentClassToType<ComponentClass>();
			if(existsBucketFor(t))
				return _componentBuckets[t];
			else
				throw std::invalid_argument("Component type " + std::to_string(t) + " not registred.");
		}

		
	private:
		SolidVector<Entity> _entities;
		std::map<ComponentType, std::unique_ptr<ComponentContainerBase>> _componentBuckets;
		std::map<std::type_index, ComponentType> _componentClassToType;


		Component::ID addComponent(ComponentType t) {
			if(existsBucketFor(t))
				return _componentBuckets[t]->emplace();
			else
				throw std::invalid_argument("Component type " + std::to_string(t) + " not registred.");
		}

		ComponentBase* getComponent(ComponentType t, Component::ID id) {
			if(existsBucketFor(t))
				return _componentBuckets[t]->get(id);
			else
				throw std::invalid_argument("Component type " + std::to_string(t) + " not registred.");
		}

		void removeComponent(ComponentType t, Component::ID id) {
			if(existsBucketFor(t))
				_componentBuckets[t]->remove(id);
		}

		bool existsBucketFor(ComponentType t) {
			if(_componentBuckets.find(t) != _componentBuckets.end())
				return true;
			else
				return false;
		}

		template <typename ComponentClass>
		ComponentType componentClassToType() {
			if(_componentClassToType.find(typeid(ComponentClass)) != _componentClassToType.end())
				return _componentClassToType[typeid(ComponentClass)];
			else
				throw std::invalid_argument("Component class " + std::string(typeid(ComponentClass).name()) + " not registered.");
		}
};
#endif /* ENTITYMANAGER_HPP_17_04_20_11_26_38 */
