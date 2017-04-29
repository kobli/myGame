#ifndef OBSERVABLEENTITYMANAGER_HPP_17_04_28_11_28_05
#define OBSERVABLEENTITYMANAGER_HPP_17_04_28_11_28_05 

#include "entityTemplates.hpp"
#include "observer.hpp"

template <typename ComponentType>
struct EntityEvent {
	ID entityID 							= NULLID;
	ID componentID 						= NULLID; // TODO this is maybe redundant ... but not if ComponentType does not have NULLType
	ComponentType componentT 	= ComponentType{};
	bool created 							= false;
	bool destroyed 						= false;
};


/*
 * need to observe:
 * component:
 * 	- change
 * 	- creation
 * 	- destruction
 * entity:
 * 	- creation
 * 	- destruction
 *
 * 	! it would be logical to be able to observe an entity to get changes in components
 */


template <typename ComponentBase, typename ComponentType>
class ObservableEntity : public Entity<ComponentBase,ComponentType>, public Observabler<EntityEvent<ComponentType>> {

	typedef EntityManagerBase<ComponentBase,ComponentType> EntityManagerBaseT;
	typedef Entity<ComponentBase,ComponentType> EntityBaseT;
	typedef EntityEvent<ComponentType> EventT;

	static_assert(std::is_base_of<Observable<EventT>, ComponentBase>::value, "ComponentBase must be Observable");

	public:
		ObservableEntity(EntityManagerBaseT& manager) : EntityBaseT{manager}, Observabler<EventT>(EventT(), EventT()) {
		}

		ObservableEntity(EntityBaseT&& other) noexcept : EntityBaseT{other} {
		}

		virtual void addComponent(ComponentType t) {
			EntityBaseT::addComponent(t);
			std::cout << "add component\n";
			auto* c = EntityBaseT::getComponent(t);
			assert(c != nullptr);
			//TODO .. first fix the bug - try unit test lower level parts this->observe(*c);
		}

};


template <typename ComponentBase, typename ComponentType, typename EntityT>
class ObservableEntityManager : public EntityManager<ComponentBase,ComponentType,EntityT>, public Observabler<EntityEvent<ComponentType>>{
	
	friend EntityT;
	typedef EntityManager<ComponentBase,ComponentType,EntityT> BaseEM;
	typedef EntityEvent<ComponentType> EventT;

	static_assert(std::is_base_of<Observable<EventT>, EntityT>::value, "EntityT must be Observable");

	public:
		ObservableEntityManager(): Observabler<EventT>(EventT(), EventT()) {
		}
			
		ID createEntity() {
			std::cout << "createEntity\n";
			ID id = BaseEM::createEntity();
			this->observe(*this->getEntity(id));
			return id;
		}
};
#endif /* OBSERVABLEENTITYMANAGER_HPP_17_04_28_11_28_05 */
