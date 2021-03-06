#ifndef OBSERVABLEENTITYMANAGER_HPP_17_04_28_11_28_05
#define OBSERVABLEENTITYMANAGER_HPP_17_04_28_11_28_05 
#include "entityComponent.hpp"
#include "observer.hpp"

namespace ec {

template <typename ComponentType>
struct EntityEvent {
	EntityEvent(ID eID, ComponentType compT = ComponentType::NONE, 
			bool c = false, bool d = false)
		: entityID{eID}, componentT{compT}, created{c}, destroyed{d} {
	}

	bool operator==(const EntityEvent& other) const {
		return entityID == other.entityID && componentT == other.componentT;
	}

	ID entityID;
	ComponentType componentT = ComponentType::NONE; // for entity created / destroyed - component non-related events
	bool created;
	bool destroyed;
};

template <typename CompT>
std::ostream& operator<<(std::ostream& o, const EntityEvent<CompT>& e) {
	return o << e.entityID << " " << e.componentT;
}


template <typename ComponentBase, typename ComponentType, typename EventT>
class ObservableEntity : public Entity<ComponentBase,ComponentType>, public Observabler<EventT> {

	typedef EntityManagerBase<ComponentBase,ComponentType> EntityManagerBaseT;
	typedef Entity<ComponentBase,ComponentType> EntityBaseT;

	static_assert(std::is_base_of<Observable<EventT>, ComponentBase>::value, "ComponentBase must be Observable");

	public:
		ObservableEntity(EntityManagerBaseT& manager, ID id) : EntityBaseT{manager, id}
		, Observabler<EventT>(
				EventT{id, ComponentType::NONE, true}, 
				EventT{id, ComponentType::NONE, false, true}) {
		}

		virtual void afterAddComponent(ComponentType t) override {
			auto* c = static_cast<ComponentBase*>(EntityBaseT::getComponent(t));
			assert(c != nullptr || !t);
			if(c) {
				c->addObserver(*this); 
			}
		}

		void swap(ObservableEntity& other) {
			EntityBaseT::swap(other);
			Observabler<EventT>::swap(other);
		}

		using EntityBaseT::addComponent;
		using EntityBaseT::removeComponent;
		using EntityBaseT::getComponent;
		using EntityBaseT::hasComponent;
};

template <typename T, typename TT, typename TTT>
void swap(ObservableEntity<T, TT, TTT>& lhs, ObservableEntity<T, TT, TTT>& rhs) {
	lhs.swap(rhs);
}


template <typename ComponentBase, typename ComponentType, typename EntityT, typename EventT>
class ObservableEntityManager : public EntityManager<ComponentBase,ComponentType,EntityT>, public Observabler<EventT>{
	
	friend EntityT;
	typedef EntityManager<ComponentBase,ComponentType,EntityT> BaseEM;

	static_assert(std::is_base_of<Observable<EventT>, EntityT>::value, "EntityT must be Observable");

	public:
		ObservableEntityManager(ID firstFree = ID{}): BaseEM(firstFree) {}
		virtual ID createEntity(ID hintID = NULLID) override {
			ID id = BaseEM::createEntity(hintID);
			this->getEntity(id)->addObserver(*this);
			return id;
		}
		virtual void removeEntity(ID eid) override {
			BaseEM::removeEntity(eid);
			this->broadcastMsg(EventT(eid, ComponentType::NONE, false, true));
		}
		virtual void onMessage(const EventT& e) {
			if(!(e.componentT == ComponentType::NONE && e.destroyed))
				this->broadcastMsg(e);
		}
};

}
#endif /* OBSERVABLEENTITYMANAGER_HPP_17_04_28_11_28_05 */
