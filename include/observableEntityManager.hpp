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
class ObservableEntityManager : public EntityManager<ComponentBase,ComponentType>, public Observabler<EntityEvent<ComponentType>>{
	//TODO on entityAdd/remove call observe(...)
	
	friend Entity<ComponentBase, ComponentType>;
	typedef EntityManager<ComponentBase,ComponentType> BaseEM;
	typedef EntityEvent<ComponentType> EventT;

	static_assert(std::is_base_of<Observable<EventT>, ComponentBase>::value, "ComponentBase must be Observable");

	public:
		ObservableEntityManager(): Observabler<EventT>(EventT(), EventT()) {
		}
			
		ID createEntity() {
			return BaseEM::createEntity();
		}

		Entity<ComponentBase, ComponentType>* getEntity(ID eid) {
			return BaseEM::getEntity(eid);
		}

		void removeEntity(ID eid) {
			BaseEM::removeEntity(eid);
		}

		template <typename ComponentClass>
		void registerComponentType(ComponentType t) {
			BaseEM::template registerComponentType<ComponentClass>(t);
		}

		template <typename ComponentClass>
		ComponentContainer<ComponentClass, ComponentBase, ComponentType>& getComponentBucket() {
			return BaseEM::template getComponentBucket<ComponentClass>();
		}
};

#endif /* OBSERVABLEENTITYMANAGER_HPP_17_04_28_11_28_05 */



