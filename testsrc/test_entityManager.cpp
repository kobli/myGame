#include "gtest/gtest.h"
#include "entityComponent.hpp"
enum ComponentType {
	t1,
	t2,
};

class ComponentBase {
};

class Component1 : public ComponentBase {
	public:
		int get42() {
			return 42;
		}
};

class Component2 : public ComponentBase {
};


using namespace std;

template <typename T>
void ignoreUnused(T&) {
}

typedef Entity<ComponentBase,ComponentType> EntityT;
typedef EntityManager<ComponentBase,ComponentType,EntityT> EntityManagerT;


TEST(EntityManager, getNonExistingEntity) {
	EntityManagerT em;
	ASSERT_EQ(em.getEntity(0), nullptr);
}

TEST(EntityManager, createEntity) {
	EntityManagerT em;
	em.createEntity();
}

TEST(EntityManager, getEntity) {
	EntityManagerT em;
	ID eID = em.createEntity();
	ASSERT_EQ(eID, 0);
	auto* e = em.getEntity(eID);
	ASSERT_NE(e, nullptr);
}

///////////////////// component tests ///////////////////////

TEST(EntityManager, registerComponentTypeEntityEmpty) {
	EntityManagerT em;
	em.registerComponentType<Component1>(ComponentType::t1);
	ID eID = em.createEntity();
	auto& e = *em.getEntity(eID); // should not return nullptr now

	ASSERT_EQ(e.hasComponent(ComponentType::t1), false);
}

TEST(EntityManager, createComponent) {
	EntityManagerT em;
	em.registerComponentType<Component1>(ComponentType::t1);

	ID eID = em.createEntity();
	auto& e = *em.getEntity(eID); // should not return nullptr now

	e.addComponent(ComponentType::t1);
	ASSERT_EQ(e.hasComponent(ComponentType::t1), true);
}

TEST(EntityManager, componentAccessTemplated) {
	EntityManagerT em;
	em.registerComponentType<Component1>(ComponentType::t1);

	ID eID = em.createEntity();
	auto& e = *em.getEntity(eID); // should not return nullptr now
	e.addComponent<Component1>();
	ASSERT_EQ(e.hasComponent<Component1>(), true);
}

TEST(EntityManager, removeComponent) {
	EntityManagerT em;
	em.registerComponentType<Component1>(ComponentType::t1);

	ID eID = em.createEntity();
	auto& e = *em.getEntity(eID); // should not return nullptr now

	e.removeComponent(ComponentType::t1);
	ASSERT_EQ(e.hasComponent(ComponentType::t1), false);
	ASSERT_EQ(e.getComponent(ComponentType::t1), nullptr);
}
