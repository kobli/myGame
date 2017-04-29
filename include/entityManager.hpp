#ifndef ENTITYMANAGER_HPP_17_04_20_11_26_38
#define ENTITYMANAGER_HPP_17_04_20_11_26_38 
#include <vector>
#include "solidVector.hpp"
#include "entity.hpp"
#include <memory>
#include <map>
#include <typeindex>
#include <typeinfo>
#include <iostream>


template <typename ComponentBase, typename ComponentType, typename EntityT>
class EntityManager : public EntityManagerBase<ComponentBase,ComponentType> {
	//typedef Entity<ComponentBase, ComponentType> EntityT;
	friend EntityT;

	protected:
	class ComponentContainerBase {
		public:
			virtual ID emplace() = 0;
			virtual ComponentBase* get(ID cid) = 0;
			virtual void remove(ID cid) = 0;
	};
	public:
	template <typename T>
		class ComponentContainer : public ComponentContainerBase {

			public:
			typedef typename SolidVector<T>::iterator iterator;

			iterator begin() {
				return _vec.begin();
			}

			iterator end() {
				return _vec.end();
			}

			private:
			virtual ID emplace() {
				return _vec.emplace();
			}

			virtual ComponentBase* get(ID cid) {
				if(_vec.indexValid(cid))
					return &_vec[cid];
				else
					return nullptr;
			}

			virtual void remove(ID cid) {
				_vec.remove(cid);
			}

			private:
			SolidVector<T> _vec;
		};


	public:
		ID createEntity() {
			return _entities.insert(EntityT(*this));
		}

		EntityT* getEntity(ID eid) {
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
		SolidVector<EntityT> _entities;
		std::map<ComponentType, std::unique_ptr<ComponentContainerBase>> _componentBuckets;
		std::map<std::type_index, ComponentType> _componentClassToType;


		virtual ID addComponent(ComponentType t) {
			if(existsBucketFor(t))
				return _componentBuckets[t]->emplace();
			else
				throw std::invalid_argument("Component type " + std::to_string(t) + " not registred.");
		}

		virtual ComponentBase* getComponent(ComponentType t, ID id) {
			if(existsBucketFor(t))
				return _componentBuckets[t]->get(id);
			else
				throw std::invalid_argument("Component type " + std::to_string(t) + " not registred.");
		}

		virtual void removeComponent(ComponentType t, ID cid) {
			std::cout << "EM::removeComponent\n";
			if(existsBucketFor(t) && _componentBuckets[t].get())
				_componentBuckets[t]->remove(cid);
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
