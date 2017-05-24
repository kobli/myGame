#ifndef OBSERVABLEENTITYMANAGER_HPP_17_04_28_11_28_05
#define OBSERVABLEENTITYMANAGER_HPP_17_04_28_11_28_05 
#include "entityComponent.hpp"
#include "observer.hpp"

template <typename ComponentType>
struct EntityEvent {
	ID entityID;
	ComponentType componentT 	= ComponentType::NONE; // for entity created / destroyed - component non-related events
};


template <typename ComponentBase, typename ComponentType>
class ObservableEntity : public Entity<ComponentBase,ComponentType>, public Observabler<EntityEvent<ComponentType>> {

	typedef EntityManagerBase<ComponentBase,ComponentType> EntityManagerBaseT;
	typedef Entity<ComponentBase,ComponentType> EntityBaseT;
	typedef EntityEvent<ComponentType> EventT;

	static_assert(std::is_base_of<Observable<EventT>, ComponentBase>::value, "ComponentBase must be Observable");

	public:
		ObservableEntity(EntityManagerBaseT& manager) : EntityBaseT{manager}, Observabler<EventT>() {
		}

		ObservableEntity(EntityBaseT&& other) noexcept : EntityBaseT{other} {
		}

		virtual void addComponent(ComponentType t) {
			EntityBaseT::addComponent(t);
			auto* c = static_cast<ComponentBase*>(EntityBaseT::getComponent(t));
			assert(c != nullptr);
			c->addObserver(*this); 
		}

		using EntityBaseT::addComponent;
		using EntityBaseT::removeComponent;
		using EntityBaseT::getComponent;
		using EntityBaseT::hasComponent;
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
			ID id = BaseEM::createEntity();
			this->getEntity(id)->addObserver(*this);
			return id;
		}
};
#endif /* OBSERVABLEENTITYMANAGER_HPP_17_04_28_11_28_05 */
