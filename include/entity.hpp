#ifndef ENTITY_HPP_17_04_20_11_22_33
#define ENTITY_HPP_17_04_20_11_22_33 

#include <array>
#include <vector>
#include <map>

typedef uint16_t ID;
const ID NULLID = ID{}-1;

template <typename ComponentBase, typename ComponentType>
class EntityManagerBase {
	public:
		virtual ID addComponent(ComponentType t) = 0;
		virtual ComponentBase* getComponent(ComponentType t, ID id) = 0;
		virtual void removeComponent(ComponentType t, ID id) = 0;
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

		/*
		template<typename T>
		void addComponent();
		template<typename T>
		void removeComponent();
		template<typename T>
		T* getComponent();
		template<typename T>
		bool hasComponent();
		*/

	private:
		EntityManagerBaseT* _manager;
		std::map<ComponentType,ID> _componentID;
};

template <typename T, typename TT>
void swap(Entity<T, TT>& lhs, Entity<T, TT>& rhs) {
	lhs.swap(rhs);
}

#endif /* ENTITY_HPP_17_04_20_11_22_33 */
