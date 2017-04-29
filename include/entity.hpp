#ifndef ENTITY_HPP_17_04_20_11_22_33
#define ENTITY_HPP_17_04_20_11_22_33 

#include <vector>
#include <memory>
#include <array>
#include <map>
#include "solidVector.hpp"
#include <typeindex>
#include <typeinfo>
#include <iostream>

typedef uint16_t ID;
const ID NULLID = ID{}-1;

template <typename ComponentBase, typename ComponentType>
class EntityManagerBase {
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

	protected:
		std::map<ComponentType, std::unique_ptr<ComponentContainerBase>> _componentBuckets;
		std::map<std::type_index, ComponentType> _componentClassToType;


	public:
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

template <typename ComponentBase, typename ComponentType>
class Entity {
	typedef EntityManagerBase<ComponentBase,ComponentType> EntityManagerBaseT;

	public:
		Entity(EntityManagerBaseT& manager);

		Entity(Entity&& other) noexcept : _manager{other._manager} {
			swap(other);
		}

		~Entity() noexcept {
			for(auto& v : _componentID)
				removeComponent(v.first);
		}

		void swap(Entity& other) {
			using std::swap;
			swap(_manager, other._manager);
			swap(_componentID, other._componentID);
		}


		virtual void addComponent(ComponentType t);
		void removeComponent(ComponentType t);
		ComponentBase* getComponent(ComponentType t);
		bool hasComponent(ComponentType t);

		template<typename T>
		void addComponent();
		template<typename T>
		void removeComponent();
		template<typename T>
		T* getComponent();
		template<typename T>
		bool hasComponent();

	private:
		EntityManagerBaseT* _manager;
		std::map<ComponentType,ID> _componentID;
};

template <typename T, typename TT>
void swap(Entity<T, TT>& lhs, Entity<T, TT>& rhs) {
	lhs.swap(rhs);
}

#endif /* ENTITY_HPP_17_04_20_11_22_33 */
