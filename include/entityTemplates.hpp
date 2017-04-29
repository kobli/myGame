#ifndef ENTITYTEMPLATES_HPP_17_04_26_12_30_15
#define ENTITYTEMPLATES_HPP_17_04_26_12_30_15 
#include "entityManager.hpp"

template <typename ComponentBase, typename ComponentType>
void Entity<ComponentBase,ComponentType>::addComponent(ComponentType t) {
	if(!hasComponent(t))
		_componentID[t] = _manager->addComponent(t);
}

template <typename ComponentBase, typename ComponentType>
void Entity<ComponentBase,ComponentType>::removeComponent(ComponentType t) {
	if(hasComponent(t)) {
		_manager->removeComponent(t, _componentID[t]);
		_componentID.erase(t);
		}
}

template <typename ComponentBase, typename ComponentType>
Entity<ComponentBase,ComponentType>::Entity(EntityManagerBaseT& manager) : _manager{&manager} {
}

template <typename ComponentBase, typename ComponentType>
ComponentBase* Entity<ComponentBase,ComponentType>::getComponent(ComponentType t) {
	if(hasComponent(t))
		return _manager->getComponent(t, _componentID[t]);
	else
		return nullptr;
}

template <typename ComponentBase, typename ComponentType>
bool Entity<ComponentBase,ComponentType>::hasComponent(ComponentType t) {
	return _componentID.find(t) != _componentID.end();
}

/*
template <typename ComponentBase, typename ComponentType>
template<typename T>
void Entity<ComponentBase,ComponentType>::addComponent() {
	addComponent(_manager->template componentClassToType<T>());
}

template <typename ComponentBase, typename ComponentType>
template<typename T>
void Entity<ComponentBase,ComponentType>::removeComponent() {
	removeComponent(_manager->template componentClassToType<T>());
}

template <typename ComponentBase, typename ComponentType>
template<typename T>
T* Entity<ComponentBase,ComponentType>::getComponent() {
	return static_cast<T*>(getComponent(_manager->template componentClassToType<T>()));
}

template <typename ComponentBase, typename ComponentType>
template<typename T>
bool Entity<ComponentBase,ComponentType>::hasComponent() {
	return hasComponent(_manager->template componentClassToType<T>());
}
*/
#endif /* ENTITYTEMPLATES_HPP_17_04_26_12_30_15 */
