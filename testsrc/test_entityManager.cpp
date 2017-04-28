#include "gtest/gtest.h"
#include "entityTemplates.hpp"
enum ComponentType {
	t1,
	t2,
};

class ComponentBase {
};

class Component1 : public ComponentBase {
};

class Component2 : public ComponentBase {
};


using namespace std;

template <typename T>
void ignoreUnused(T&) {
}


TEST(EntityManager, getNonExistingEntity) {
	EntityManager<ComponentBase,ComponentType> em;
	ASSERT_EQ(em.getEntity(0), nullptr);
}

TEST(EntityManager, createEntity) {
	EntityManager<ComponentBase,ComponentType> em;
	em.createEntity();
}

TEST(EntityManager, getEntity) {
	EntityManager<ComponentBase,ComponentType> em;
	ID eID = em.createEntity();
	ASSERT_EQ(eID, 0);
	auto* e = em.getEntity(eID);
	ASSERT_NE(e, nullptr);
}

///////////////////// component tests ///////////////////////

TEST(EntityManager, registerComponentType) {
	EntityManager<ComponentBase,ComponentType> em;
	em.registerComponentType<Component1>(ComponentType::t1);
}

TEST(EntityManager, createComponent) {
	EntityManager<ComponentBase,ComponentType> em;
	em.registerComponentType<Component1>(ComponentType::t1);

	ID eID = em.createEntity();
	auto& e = *em.getEntity(eID); // should not return nullptr now

	ASSERT_EQ(e.hasComponent(ComponentType::t1), false);
	//e.addComponent(ComponentType::t1);
	e.addComponent<Component1>();
	//ASSERT_EQ(true, e.hasComponent(ComponentType::t1));
	ASSERT_EQ(e.hasComponent<Component1>(), true);
	Component1& c1 = *e.getComponent<Component1>(); // should not return nullptr now
	ignoreUnused(c1);
}
