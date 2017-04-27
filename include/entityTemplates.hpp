#ifndef ENTITYTEMPLATES_HPP_17_04_26_12_30_15
#define ENTITYTEMPLATES_HPP_17_04_26_12_30_15 
#include "entityManager.hpp"

template<typename T>
void Entity::addComponent() {
	addComponent(_manager->componentClassToType<T>());
}

template<typename T>
void Entity::removeComponent() {
	removeComponent(_manager->componentClassToType<T>());
}

template<typename T>
T* Entity::getComponent() {
	return static_cast<T*>(getComponent(_manager->componentClassToType<T>()));
}

template<typename T>
bool Entity::hasComponent() {
	return hasComponent(_manager->componentClassToType<T>());
}
#endif /* ENTITYTEMPLATES_HPP_17_04_26_12_30_15 */
