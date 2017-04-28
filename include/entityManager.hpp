#ifndef ENTITYMANAGER_HPP_17_04_20_11_26_38
#define ENTITYMANAGER_HPP_17_04_20_11_26_38 
#include <vector>
#include "solidVector.hpp"
#include "entity.hpp"
#include <memory>
#include <map>
#include <typeindex>
#include <typeinfo>

template <typename ComponentBase>
class ComponentContainerBase {
	public:
		virtual ID emplace() = 0;
		virtual ComponentBase* get(ID cid) = 0;
		virtual void remove(ID cid) = 0;
};


template <typename T, typename ComponentBase, typename ComponentType>
class ComponentContainer : public ComponentContainerBase<ComponentBase> {
	friend EntityManager<ComponentBase, ComponentType>;

	public:
		typedef typename SolidVector<T>::iterator iterator;

		iterator begin() {
			return _vec.begin();
		}

		iterator end() {
			return _vec.end();
		}

	protected:
		ID emplace() {
			return _vec.emplace();
		}

		ComponentBase* get(ID cid) {
			if(_vec.indexValid(cid))
				return &_vec[cid];
			else
				return nullptr;
		}

		void remove(ID cid) {
			_vec.remove(cid);
		}

	private:
		SolidVector<T> _vec;
};


template <typename ComponentBase, typename ComponentType>
class EntityManager {
	friend Entity<ComponentBase, ComponentType>;

	public:
		ID createEntity() {
			return _entities.insert(Entity<ComponentBase, ComponentType>(*this));
		}

		Entity<ComponentBase, ComponentType>* getEntity(ID eid) {
			if(_entities.indexValid(eid))
				return &_entities[eid];
			else
				return nullptr;
		}

		void removeEntity(ID eid) {
			if(_entities.indexValid(eid))
				_entities.remove(eid);
		}

		template <typename ComponentClass>
		void registerComponentType(ComponentType t) {
			if(!existsBucketFor(t)) {
				_componentBuckets[t] = std::make_unique<ComponentContainer<ComponentClass, ComponentBase, ComponentType>>();
				_componentClassToType[typeid(ComponentClass)] = t;
			}
			else
				throw std::invalid_argument("Component class " + std::string(typeid(ComponentClass).name()) + " is already registered.");
		}

		template <typename ComponentClass>
		ComponentContainer<ComponentClass, ComponentBase, ComponentType>& getComponentBucket() {
			ComponentType t = componentClassToType<ComponentClass>();
			if(existsBucketFor(t))
				return _componentBuckets[t];
			else
				throw std::invalid_argument("Component type " + std::to_string(t) + " not registred.");
		}

		
	private:
		SolidVector<Entity<ComponentBase, ComponentType>> _entities;
		std::map<ComponentType, std::unique_ptr<ComponentContainerBase<ComponentBase>>> _componentBuckets;
		std::map<std::type_index, ComponentType> _componentClassToType;


		ID addComponent(ComponentType t) {
			if(existsBucketFor(t))
				return _componentBuckets[t]->emplace();
			else
				throw std::invalid_argument("Component type " + std::to_string(t) + " not registred.");
		}

		ComponentBase* getComponent(ComponentType t, ID id) {
			if(existsBucketFor(t))
				return _componentBuckets[t]->get(id);
			else
				throw std::invalid_argument("Component type " + std::to_string(t) + " not registred.");
		}

		void removeComponent(ComponentType t, ID id) {
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
