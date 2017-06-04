#include "gtest/gtest.h"
#include "entityComponent.hpp"
enum ComponentType {
	NONE,
	t1,
	t2,
};

using ec::ID;
using ec::NULLID;

class ComponentBase {
};

class Component1 : public ComponentBase {
	public:
		Component1(ID parentEntID) {
		}

		int get42() {
			return 42;
		}
};

class Component2 : public ComponentBase {
	public:
	Component2(ID parentEntID) {
	}
};


using namespace std;

template <typename T>
void ignoreUnused(T&) {
}

typedef ec::Entity<ComponentBase,ComponentType> Entity;
typedef ec::EntityManager<ComponentBase,ComponentType,Entity> EntityManager;


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
	ID eID = em.createEntity();
	ASSERT_EQ(eID, 0);
	auto* e = em.getEntity(eID);
	ASSERT_NE(e, nullptr);
}

///////////////////// component tests ///////////////////////

TEST(EntityManager, registerComponentTypeEntityEmpty) {
	EntityManager em;
	em.registerComponentType<Component1>(ComponentType::t1);
	ID eID = em.createEntity();
	auto& e = *em.getEntity(eID); // should not return nullptr now

	ASSERT_EQ(e.hasComponent(ComponentType::t1), false);
}

TEST(EntityManager, createComponent) {
	EntityManager em;
	em.registerComponentType<Component1>(ComponentType::t1);

	ID eID = em.createEntity();
	auto& e = *em.getEntity(eID); // should not return nullptr now

	e.addComponent(ComponentType::t1);
	ASSERT_EQ(e.hasComponent(ComponentType::t1), true);
}

TEST(EntityManager, componentAccessTemplated) {
	EntityManager em;
	em.registerComponentType<Component1>(ComponentType::t1);

	ID eID = em.createEntity();
	auto& e = *em.getEntity(eID); // should not return nullptr now
	e.addComponent<Component1>();
	ASSERT_EQ(e.hasComponent<Component1>(), true);
}

TEST(EntityManager, removeComponent) {
	EntityManager em;
	em.registerComponentType<Component1>(ComponentType::t1);

	ID eID = em.createEntity();
	auto& e = *em.getEntity(eID); // should not return nullptr now

	e.removeComponent(ComponentType::t1);
	ASSERT_EQ(e.hasComponent(ComponentType::t1), false);
	ASSERT_EQ(e.getComponent(ComponentType::t1), nullptr);
}
