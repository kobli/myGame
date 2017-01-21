#include <sceneNodeManager.hpp>

SceneNodeManager::SceneNodeManager(irr::scene::ISceneManager* smgr, World& world): _smgr{smgr}, _world{world}
{
	observe(_world);
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
				//sn->updateAbsolutePosition();
				sn->setName("graphics");
				sn->setDebugDataVisible(scene::EDS_FULL);
				break;
			}
		default:
			break;
	}
}

void SceneNodeManager::onObservableRemove(EntityEvent&)
{}
