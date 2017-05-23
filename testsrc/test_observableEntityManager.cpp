#include "gtest/gtest.h"
#include "observableEntityManager.hpp"
enum ComponentType {
	t1,
	t2,
};

class ObservableComponentBase : public Observable<EntityEvent<ComponentType>> {
	typedef EntityEvent<ComponentType> EventT;
	public:
	ObservableComponentBase() : Observable<EventT>{EventT{}, EventT{}} {
	}
};

class ObservableComponent1 : public ObservableComponentBase {
};

class ObservableComponent2 : public ObservableComponentBase {
};


using namespace std;

template <typename T>
void ignoreUnused(T&) {
}

typedef ObservableEntity<ObservableComponentBase,ComponentType> EntityT;
typedef ObservableEntityManager<ObservableComponentBase,ComponentType,EntityT> EntityManagerT;


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
