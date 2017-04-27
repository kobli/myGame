#ifndef ENTITY_HPP_17_04_20_11_22_33
#define ENTITY_HPP_17_04_20_11_22_33 

#include "component.hpp"
#include <array>
#include <vector>
#include <map>

class EntityManager;

class Entity {
	friend EntityManager;

	public:
		typedef unsigned ID;
		static const ID NULLID;

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
		EntityManager* _manager;
		std::map<ComponentType,ID> _componentID;

		Entity(EntityManager& manager);
};

void swap(Entity& lhs, Entity& rhs);

#endif /* ENTITY_HPP_17_04_20_11_22_33 */
