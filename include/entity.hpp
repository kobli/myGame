#ifndef ENTITY_HPP_17_04_20_11_22_33
#define ENTITY_HPP_17_04_20_11_22_33 

#include <array>
#include <vector>
#include <map>

typedef uint16_t ID;
const ID NULLID = ID{}-1;


template <typename ComponentBase, typename ComponentType>
class EntityManager;


template <typename ComponentBase, typename ComponentType>
class Entity {
	friend EntityManager<ComponentBase, ComponentType>;

	public:

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


		void addComponent(ComponentType t);
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
		EntityManager<ComponentBase, ComponentType>* _manager;
		std::map<ComponentType,ID> _componentID;

		Entity(EntityManager<ComponentBase, ComponentType>& manager);
};

template <typename T, typename TT>
void swap(Entity<T, TT>& lhs, Entity<T, TT>& rhs) {
	lhs.swap(rhs);
}

#endif /* ENTITY_HPP_17_04_20_11_22_33 */
