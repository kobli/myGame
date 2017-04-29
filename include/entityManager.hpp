#ifndef ENTITYMANAGER_HPP_17_04_20_11_26_38
#define ENTITYMANAGER_HPP_17_04_20_11_26_38 
#include "solidVector.hpp"
#include "entity.hpp"


template <typename ComponentBase, typename ComponentType, typename EntityT>
class EntityManager : public EntityManagerBase<ComponentBase,ComponentType> {
	//typedef Entity<ComponentBase, ComponentType> EntityT;
	friend EntityT;


	public:
		ID createEntity() {
			return _entities.insert(EntityT(*this));
		}

		EntityT* getEntity(ID eid) {
			if(_entities.indexValid(eid))
				return &_entities[eid];
			else
				return nullptr;
		}

		void removeEntity(ID eid) {
			if(_entities.indexValid(eid))
				_entities.remove(eid);
		}

	private:
		SolidVector<EntityT> _entities;
};
#endif /* ENTITYMANAGER_HPP_17_04_20_11_26_38 */
