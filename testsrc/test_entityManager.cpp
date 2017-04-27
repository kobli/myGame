#include "gtest/gtest.h"
#include "entityManager.hpp"
#include "entityTemplates.hpp"

using namespace std;

template <typename T>
void ignoreUnused(T&) {
}


TEST(EntityManager, getNonExistingEntity) {
	EntityManager em;
	ASSERT_EQ(em.getEntity(0), nullptr);
}

TEST(EntityManager, createEntity) {
	EntityManager em;
	em.createEntity();
}

TEST(EntityManager, getEntity) {
	EntityManager em;
	Entity::ID eID = em.createEntity();
	ASSERT_EQ(eID, 0);
	Entity* e = em.getEntity(eID);
	ASSERT_NE(e, nullptr);
}

///////////////////// component tests ///////////////////////

TEST(EntityManager, registerComponentType) {
	EntityManager em;
	em.registerComponentType<Component1>(ComponentType::t1);
}

TEST(EntityManager, createComponent) {
	EntityManager em;
	em.registerComponentType<Component1>(ComponentType::t1);

	Entity::ID eID = em.createEntity();
	Entity& e = *em.getEntity(eID); // should not return nullptr now

	ASSERT_EQ(e.hasComponent(ComponentType::t1), false);
	//e.addComponent(ComponentType::t1);
	e.addComponent<Component1>();
	//ASSERT_EQ(true, e.hasComponent(ComponentType::t1));
	ASSERT_EQ(e.hasComponent<Component1>(), true);
	Component1& c1 = *e.getComponent<Component1>(); // should not return nullptr now
	ignoreUnused(c1);
}
