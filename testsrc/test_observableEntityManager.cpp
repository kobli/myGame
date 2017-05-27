#include "gtest/gtest.h"
#include "observableEntityComponent.hpp"
#include "observerMock.hpp"

enum ComponentType {
	NONE,
	t1,
	t2,
};

class ObservableComponentBase : public Observable<EntityEvent<ComponentType>> {
	protected:
	typedef EntityEvent<ComponentType> EventT;
	public:
	ObservableComponentBase(const EventT& addEvent, const EventT& remEvent) : Observable<EventT>{addEvent, remEvent} {
	}
};

class ObservableComponent1 : public ObservableComponentBase {
	public:
	ObservableComponent1(ID parentEntID) : ObservableComponentBase(EventT{parentEntID, t1}, EventT{parentEntID, t1}) {
	}
};

class ObservableComponent2 : public ObservableComponentBase {
	public:
	ObservableComponent2(ID parentEntID) : ObservableComponentBase(EventT{parentEntID, t2}, EventT{parentEntID, t2}) {
	}
};


template <typename T>
void ignoreUnused(T&) {
}

typedef ObservableEntity<ObservableComponentBase,ComponentType> EntityT;
typedef ObservableEntityManager<ObservableComponentBase,ComponentType,EntityT> EntityManagerT;

typedef EntityEvent<ComponentType> EventT;
typedef ObserverMock_<EventT> ObserverMock;
typedef ObserverMock::MsgSeqCont MsgSeq;


TEST(ObservableEntityManager, getNonExistingEntity) {
	EntityManagerT em;
	ASSERT_EQ(em.getEntity(0), nullptr);
}

TEST(ObservableEntityManager, createEntity) {
	EntityManagerT em;
	em.createEntity();
}

TEST(ObservableEntityManager, getEntity) {
	EntityManagerT em;
	ID eID = em.createEntity();
	ASSERT_EQ(eID, 0);
	auto* e = em.getEntity(eID);
	ASSERT_NE(e, nullptr);
}

///////////////////// component tests ///////////////////////

TEST(ObservableEntityManager, registerComponentType) {
	EntityManagerT em;
	em.registerComponentType<ObservableComponent1>(ComponentType::t1);
}

TEST(ObservableEntityManager, createComponent) {
	EntityManagerT em;
	em.registerComponentType<ObservableComponent1>(ComponentType::t1);

	ID eID = em.createEntity();
	auto& e = *em.getEntity(eID); // should not return nullptr now

	//e.addComponent<ObservableComponent1>();
	e.addComponent(ComponentType::t1);
	ASSERT_EQ(e.hasComponent(ComponentType::t1), true);
}

TEST(ObservableEntityManager, removeComponent) {
	EntityManagerT em;
	em.registerComponentType<ObservableComponent1>(ComponentType::t1);

	ID eID = em.createEntity();
	auto& e = *em.getEntity(eID); // should not return nullptr now

	e.removeComponent(ComponentType::t1);
	ASSERT_EQ(e.hasComponent(ComponentType::t1), false);
	ASSERT_EQ(e.getComponent(ComponentType::t1), nullptr);
}

TEST(ObservableEntityManager, messages) {
	// on EntityManager death the observer should receive remMsg from each tree node 
	ObserverMock observer(MsgSeq{
			// add events
			EventT{0},
			EventT{0, ComponentType::t1},

			//remove events
			EventT{0},
			EventT{0, ComponentType::t1},
			});
	EntityManagerT em;
	em.registerComponentType<ObservableComponent1>(ComponentType::t1);

	em.addObserver(observer);

	ID eID = em.createEntity();
	auto& e = *em.getEntity(eID); // should not return nullptr now

	e.addComponent(ComponentType::t1);
}
