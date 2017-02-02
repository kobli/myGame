/*
#include <sceneNodeManager.hpp>

SceneNodeManager::SceneNodeManager(irr::scene::ISceneManager* smgr, World& world): _smgr{smgr}, _world{world}
{
	observe(_world);
//	_worldTS = _world.getMap().getMetaTriangleSelector(); //TODO this will make mess when there are multiple worlds at the same time
}

vec3f SceneNodeManager::getEntityResultPos(WorldEntity& e, vec3f posDiff)
{
	scene::ISceneNode* sn = _smgr->getSceneNodeFromId(e.getID());
	if(sn)
	{
		// try to set position of the entitys sceneNode to the requested position
		// it will adjust itself depending on the collision component and callback
		auto oldP = e.getBodyComponent()->getPosition();
		auto newP = oldP*vec3f(1,0,1) + sn->getPosition()*vec3f(0,1,0) + posDiff;
		//std::cout << "delta: " << abs(newP.getDistanceFrom(oldP)) << std::endl;
		if(abs(newP.getDistanceFrom(oldP)) < 0.1)
			newP = oldP;
		sn->setPosition(newP);
		
		return sn->getPosition();
	}
	return e.getBodyComponent()->getPosition();
}

void SceneNodeManager::onObservableAdd(EntityEvent&)
{}

void SceneNodeManager::onObservableUpdate(EntityEvent& m)
{
	switch(m._componentModifiedType)
	{
		case ComponentType::Body:
			{
				scene::ISceneNode* sn = _smgr->getSceneNodeFromId(m._entityID);
				if(m._destroyed && sn)
				{
					sn->remove();
					return;
				}
				if(!sn)
					sn = _smgr->addEmptySceneNode(nullptr, m._entityID);
				sn->setPosition(static_cast<BodyComponent*>(m._componentModified)->getPosition());
				sn->updateAbsolutePosition();
				sn->setName("body");
				sn->setDebugDataVisible(scene::EDS_FULL);
				break;
			}
		case ComponentType::GraphicsSphere:
			{
				auto e = _world.getEntityByID(m._entityID);
				if(!e)
					return;
				auto bc = e->getBodyComponent();
				if(!bc)
					return;
				auto bsn = _smgr->getSceneNodeFromId(m._entityID);
				if(!bsn)
					std::cout << "BSN NOT FOUND\n";
				auto sn = _smgr->getSceneNodeFromName("graphics", bsn);
				if(sn)
					sn->remove();
				if(m._destroyed)
					return;
				std::cout << "graphics\n";
				std::cout << "radius: " << static_cast<SphereGraphicsComponent*>(m._componentModified)->getRadius() << std::endl;
				auto gc = static_cast<SphereGraphicsComponent*>(m._componentModified);
				sn = _smgr->addSphereSceneNode(gc->getRadius(), 64, bsn, -1, gc->getPosOffset());
				sn->setName("graphics");
				sn->setDebugDataVisible(scene::EDS_FULL);
				break;
			}
		case ComponentType::Collision:
			{
				auto e = _world.getEntityByID(m._entityID);
				if(!e)
					return;
				auto bc = e->getBodyComponent();
				if(!bc)
					return;
				auto bsn = _smgr->getSceneNodeFromId(m._entityID);
				if(!bsn)
					std::cout << "BSN NOT FOUND\n";
				if(m._destroyed)
					return;
				auto cc = e->getCollisionComponent();
				vec3f colliderSize = cc->getColliderRadius();
				auto gsn = _smgr->getSceneNodeFromName("graphics", bsn);
				if(gsn)
				{
					colliderSize = gsn->getTransformedBoundingBox().MaxEdge - gsn->getTransformedBoundingBox().MinEdge;
					colliderSize /= 2;
					cc->setColliderRadius(colliderSize);
				}
				auto cra = _smgr->createCollisionResponseAnimator(_world.getMap().getMetaTriangleSelector(), bsn, colliderSize, vec3f(0,-10,0));
				cra->setCollisionCallback(this);
				bsn->addAnimator(cra);
				//auto ts = _smgr->createTriangleSelector(static_cast<scene::IMeshSceneNode*>(gsn)->getMesh(), gsn);
				//bsn->setTriangleSelector(ts);
				//_worldTS->addTriangleSelector(ts);
				break;
			}
		default:
			break;
	}
}

void SceneNodeManager::onObservableRemove(EntityEvent&)
{}

bool SceneNodeManager::onCollision(const scene::ISceneNodeAnimatorCollisionResponse& animator)
{
	//std::cout << "COLLISION\n";
	std::cout << "COL: " << animator.getCollisionNode()->getID() << std::endl;
	return false;
}
*/
