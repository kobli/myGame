#include "gtest/gtest.h"
#include "observableEntityComponent.hpp"
#include "observerMock.hpp"

enum ComponentType {
	NONE,
	t1,
	t2,
};

using ec::ID;
using ec::NULLID;
typedef ec::EntityEvent<ComponentType> Event;

class ObservableComponentBase : public Observable<Event> {
	public:
	ObservableComponentBase(const Event& addEvent, const Event& remEvent) : Observable<Event>{addEvent, remEvent} {
	}
};

class ObservableComponent1 : public ObservableComponentBase {
	public:
	ObservableComponent1(ID parentEntID) : ObservableComponentBase(Event{parentEntID, t1}, Event{parentEntID, t1}) {
	}
};

class ObservableComponent2 : public ObservableComponentBase {
	public:
	ObservableComponent2(ID parentEntID) : ObservableComponentBase(Event{parentEntID, t2}, Event{parentEntID, t2}) {
	}
};


template <typename T>
void ignoreUnused(T&) {
}

typedef ec::ObservableEntity<ObservableComponentBase,ComponentType,Event> Entity;
typedef ec::ObservableEntityManager<ObservableComponentBase,ComponentType,Entity,Event> EntityManager;

typedef ObserverMock_<Event> ObserverMock;
typedef ObserverMock::MsgSeqCont MsgSeq;


TEST(ObservableEntityManager, getNonExistingEntity) {
	EntityManager em;
	ASSERT_EQ(em.getEntity(0), nullptr);
}

TEST(ObservableEntityManager, createEntity) {
	EntityManager em;
	em.createEntity();
}

TEST(ObservableEntityManager, getEntity) {
	EntityManager em;
	ID eID = em.createEntity();
	ASSERT_EQ(eID, 0);
	auto* e = em.getEntity(eID);
	ASSERT_NE(e, nullptr);
}

///////////////////// component tests ///////////////////////

TEST(ObservableEntityManager, registerComponentType) {
	EntityManager em;
	em.registerComponentType<ObservableComponent1>(ComponentType::t1);
}

TEST(ObservableEntityManager, createComponent) {
	EntityManager em;
	em.registerComponentType<ObservableComponent1>(ComponentType::t1);

	ID eID = em.createEntity();
	auto& e = *em.getEntity(eID); // should not return nullptr now

	//e.addComponent<ObservableComponent1>();
	e.addComponent(ComponentType::t1);
	ASSERT_EQ(e.hasComponent(ComponentType::t1), true);
}

TEST(ObservableEntityManager, removeComponent) {
	EntityManager em;
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
			Event{0},
			Event{0, ComponentType::t1},

			//remove events
			Event{0},
			Event{0, ComponentType::t1},
			});
	EntityManager em;
	em.registerComponentType<ObservableComponent1>(ComponentType::t1);

	em.addObserver(observer);

	ID eID = em.createEntity();
	auto& e = *em.getEntity(eID); // should not return nullptr now

	//e.addComponent(ComponentType::t1);
	e.addComponent<ObservableComponent1>();
}
