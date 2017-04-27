#include "entity.hpp"
#include "entityManager.hpp"

const Entity::ID Entity::NULLID = -1;

void Entity::addComponent(ComponentType t) {
	if(!hasComponent(t))
		_componentID[t] = _manager->addComponent(t);
}

void Entity::removeComponent(ComponentType t) {
	if(hasComponent(t))
		_componentID.erase(t);
}

Entity::Entity(EntityManager& manager) : _manager{&manager} {
}

ComponentBase* Entity::getComponent(ComponentType t) {
	if(hasComponent(t))
		return _manager->getComponent(t, _componentID[t]);
	else
		return nullptr;
}

bool Entity::hasComponent(ComponentType t) {
	return _componentID.find(t) != _componentID.end();
}

void swap(Entity& lhs, Entity& rhs) {
	lhs.swap(rhs);
}
