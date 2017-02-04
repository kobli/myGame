#include <system.hpp>
class DebugDrawer: public btIDebugDraw
{
	public:
		DebugDrawer(scene::ISceneManager* smgr): _smgr{smgr}, _dbgMode{0}
		{
		}
		virtual void 	drawLine (const btVector3 &from, const btVector3 &to, const btVector3 &color)
		{
			_smgr->getVideoDriver()->setTransform(video::E_TRANSFORMATION_STATE::ETS_WORLD, core::IdentityMatrix);
			_smgr->getVideoDriver()->draw3DLine(core::vector3df(from.x(), from.y(), from.z()),
					core::vector3df(to.x(), to.y(), to.z()),
					video::SColorf(color.x(), color.y(), color.z()).toSColor()
					);
		}
		virtual void 	drawLine (const btVector3 &from, const btVector3 &to, const btVector3 &fromColor, const btVector3 &toColor)
		{
			drawLine(from, to, fromColor);
		}
		virtual void 	drawContactPoint (const btVector3 &PointOnB, const btVector3 &normalOnB, btScalar distance, int lifeTime, const btVector3 &color)
		{
		}
		virtual void 	reportErrorWarning (const char *warningString)
		{
		}
		virtual void 	draw3dText (const btVector3 &location, const char *textString)
		{
		}
		virtual void 	setDebugMode (int debugMode)
		{
			_dbgMode = debugMode;
		}
		virtual int 	getDebugMode () const
		{
			return _dbgMode;
		}
	protected:
		scene::ISceneManager* _smgr;
		int _dbgMode;
};


class MyMotionState : public btMotionState
{
	protected:
		std::function<WorldEntity*()> _getEntity;

	public:
		MyMotionState(std::function<WorldEntity*()> getEntity): _getEntity{getEntity}
		{}

		virtual ~MyMotionState()
		{
		}

		virtual void getWorldTransform(btTransform& worldTrans) const
		{
			auto e = _getEntity();
			if(!e)
				return;
			auto bc = e->getBodyComponent();
			if(!bc)
				return;
			auto p = bc->getPosition();
			worldTrans.setOrigin(btVector3(p.X, p.Y, p.Z));
		}

		virtual void setWorldTransform(const btTransform& worldTrans)
		{
			auto e = _getEntity();
			if(!e)
				return;
			auto bc = e->getBodyComponent();
			if(!bc)
				return;
			btQuaternion rot = worldTrans.getRotation();
			vec3f r;
			btMatrix3x3(rot).getEulerZYX(r.Z, r.Y, r.X);
			bc->setRotation(r*180/M_PI);
			
			btVector3 pos = worldTrans.getOrigin();
			bc->setPosition(vec3f(pos.x(), pos.y(), pos.z()));
		}
};


Physics::Physics(World& world, scene::ISceneManager* smgr): _world{world}
{
	observe(_world);
	//TODO cleanup
	btBroadphaseInterface* broadphase = new btDbvtBroadphase();
	btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
	btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;
	_physicsWorld.reset(new btDiscreteDynamicsWorld(dispatcher,broadphase,solver,collisionConfiguration));
	_physicsWorld->setGravity(btVector3(0,-10,0));

	// set up debug drawing
	if(smgr != nullptr)
	{
		DebugDrawer* debugDrawer = new DebugDrawer(smgr);
		_physicsWorld->setDebugDrawer(debugDrawer);
		_physicsWorld->getDebugDrawer()->setDebugMode(btIDebugDraw::DBG_MAX_DEBUG_DRAW_MODE);
	}

	WorldMap& m = _world.getMap();
	// physics setup for the terrain
	int upAxis = 1; // Y
	auto heightMap = m.getHeightMap();
	unsigned w = sqrt(m.getVertexCount());

	float min = heightMap[0];
	float max = heightMap[0];
	for(unsigned y = 0; y<w; y++)
	{
		for(unsigned x = 0; x<w; x++)
		{
			float h = heightMap[x + y*w];
			if(h > max)
				max = h;
			if(h < min)
				min = h;
		}
	}
	btHeightfieldTerrainShape* terrS = new btHeightfieldTerrainShape(w, w,
				heightMap,
				0, // ignored when using float
				min, max,
				upAxis,
				PHY_FLOAT, true);
	
	// scale
	float f = m.getPatchSize()/(w-1);
	btVector3 localScaling(f, m.getHeightScale(), f);
	terrS->setLocalScaling(localScaling);

	// set origin to middle of heightfield
	btTransform tr;
	tr.setIdentity();
	tr.setOrigin(btVector3(0,(min+max)*localScaling.y()/2.,0));

	// create ground object
	btDefaultMotionState* terrMS = new btDefaultMotionState(tr);
	btRigidBody::btRigidBodyConstructionInfo terrCI(0.0, terrMS, terrS);
	btRigidBody* terrB = new btRigidBody(terrCI);
	_physicsWorld->addRigidBody(terrB);
	terrB->setUserIndex(0);
	terrB->setAnisotropicFriction(btVector3(1,.2,1));
}

vec3f Physics::getObjVelocity(u32 ID)
{
	auto co = getCollisionObjectByID(ID);
	if(!co)
		return vec3f(0);
	auto dco = dynamic_cast<btRigidBody*>(co);
	if(!dco)
		return vec3f(0);
	auto v = dco->getLinearVelocity();
	return vec3f(v.x(), v.y(), v.z());
}

void Physics::update(float timeDelta)
{
	_physicsWorld->stepSimulation(timeDelta, 10);
	bodyDoStrafe(timeDelta);
	_physicsWorld->debugDrawWorld();
	auto t = getCollisionObjectByID(0);
	auto tr = t->getWorldTransform();
	auto o = tr.getOrigin();
	vec3f r;
	btMatrix3x3(tr.getRotation()).getEulerZYX(r.X, r.Y, r.Z);
	std::cout << "pos: " << o.getX() << " " << o.getY() << " " << o.getZ() << endl;
	std::cout << "rot: " << r << endl;
}

void Physics::bodyDoStrafe(float timeDelta)
{
	for(auto& od : _objData)
		od.second.walkTimer += timeDelta;
	//TODO do this only for the bodies that collide with terrain (could be called from collision checking)
	for(auto& e: _world.getEntities())
	{
		auto bc = e.getBodyComponent();
		if(!bc)
			continue;
		auto o = getCollisionObjectByID(e.getID());
		if(!o)
			continue;
		btRigidBody* b = dynamic_cast<btRigidBody*>(o);
		if(!b)
			continue;
		/*
		float rotSpeed = 180; // degrees per second
		vec3f rot = bc->getRotation(),
					pos = bc->getPosition();
		rot.Y = fmod(rot.Y+(rotSpeed*bc->getRotDir()*timeDelta), 360);
		bc->setRotation(rot);
		auto cc = e.getCollisionComponent();
		if(cc)
			;//TODO pos = cc->getCollisionResultPosition(timeDelta);
		else
			pos += bc->getTotalVelocity()*timeDelta;
		bc->setPosition(pos);
		*/
		float& t = _objData[e.getID()].walkTimer;
		
		if(t > 0.4)
		{
			t = 0;
			float mass2 = 10000;
			float fMul = 14000;
			{
				vec2f strDir = bc->getStrafeDir();
				vec3f rot = bc->getRotation();
				vec3f dir{strDir.X, 0, strDir.Y};
				dir.rotateYZBy(-rot.X);
				dir.rotateXZBy(-rot.Y);
				dir.rotateXYBy(-rot.Z);
				dir.normalize();
				dir.Y = 0;

				//if(receiver.IsKeyDown(irr::KEY_SPACE))
					//dir.Y = 1;

				//dynamicsWorld->removeRigidBody(b);
				//pSphereShape->calculateLocalInertia(mass2, fallInertia);
				if(dir.getLength() != 0)// && s42onGr)
				{
					b->setFriction(0.9);
					//b->setMassProps(mass, fallInertia);
					//b->applyCentralForce(btVector3(dir.X, dir.Y+0.1, dir.Z)*fMul);
					b->applyCentralForce(btVector3(dir.X, 0.16, dir.Z)*fMul);
				}
				else
				{
					t = 99999;
					b->setFriction(4);
					//b->setMassProps(mass2, fallInertia);
				}
				//b->setLinearVelocity(b->getLinearVelocity() + btVector3(dir.X*MOVEMENT_SPEED, 0, dir.Z*MOVEMENT_SPEED));
				//dynamicsWorld->addRigidBody(b);
			}
		}
	}
}

void Physics::onObservableAdd(EntityEvent&)
{}

void Physics::onObservableUpdate(EntityEvent& m)
{
	switch(m._componentModifiedType)
	{
		case ComponentType::Collision:
			{
				auto e = _world.getEntityByID(m._entityID);
				if(!e)
					break;
				auto col = e->getCollisionComponent();
				if(!col)
					break;
				btScalar mass = 80;
				btScalar iner = 1;
				btVector3 fallInertia(iner, iner, iner);
				auto eID = m._entityID;

				btCollisionObject* o = getCollisionObjectByID(eID);
				if(o)
					_physicsWorld->removeCollisionObject(o);

				if(m._destroyed)
					break;

				_objData.emplace(eID, ObjData{});
				btCollisionShape* pShape = new btCapsuleShape(col->getRadius(), col->getHeight());
				pShape->calculateLocalInertia(mass,fallInertia);
				MyMotionState* motionState = new MyMotionState([this, eID]()->WorldEntity* { return _world.getEntityByID(eID); });
				btRigidBody::btRigidBodyConstructionInfo bodyCI(mass,motionState,pShape,fallInertia);
				btRigidBody* body = new btRigidBody(bodyCI);
				_physicsWorld->addRigidBody(body);
				body->setUserIndex(eID);
				//body->setCcdMotionThreshold(1e-7);
				//body->setCcdSweptSphereRadius(0.2);
				// TODO
				// mark the sphere as kinematic (so bullet loads its updated position before each simulation step)
				//fallRigidBody->setCollisionFlags(fallRigidBody->getCollisionFlags() |	btCollisionObject::CF_KINEMATIC_OBJECT);
				//body->setActivationState(DISABLE_DEACTIVATION);
				body->setAngularFactor(btVector3(0,0,0));
				//body->setRollingFriction(0);
				break;
			}
		case ComponentType::Body:
			{
				auto e = _world.getEntityByID(m._entityID);
				if(!e)
					break;
				auto col = e->getCollisionComponent();
				if(!col)
					break;
				btCollisionObject* o = getCollisionObjectByID(m._entityID);
				if(!o)
					break;
				auto b = e->getBodyComponent();
				if(!b)
					break;
				auto rigB = dynamic_cast<btRigidBody*>(o);
				auto v = b->getVelocity();
				//if(rigB) //TODO only for ghostObjects
					//rigB->setLinearVelocity(btVector3(v.X, v.Y, v.Z));
				float rotSpeed = 3;
				rigB->setAngularVelocity(btVector3(0, b->getRotDir()*rotSpeed, 0));
				rigB->activate(true);
				break;
			}
		default:
			break;
	}
}

void Physics::onObservableRemove(EntityEvent&)
{}

btCollisionObject* Physics::getCollisionObjectByID(int entityID)
{
	for(int i = 0; i < _physicsWorld->getNumCollisionObjects(); i++)
	{
		auto obj = _physicsWorld->getCollisionObjectArray()[i];
		if(obj->getUserIndex() == entityID)
			return obj;
	}
	return nullptr;
}
		
////////////////////////////////////////////////////////////

ViewSystem::ViewSystem(irr::scene::ISceneManager* smgr, World& world): _smgr{smgr}, _world{world}
{
	observe(_world);
}

void ViewSystem::onObservableAdd(EntityEvent&)
{}

void ViewSystem::onObservableUpdate(EntityEvent& m)
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
				auto e = _world.getEntityByID(m._entityID);
				if(!e)
					return;
				auto bc = e->getBodyComponent();
				if(!bc)
					return;
				sn->setPosition(bc->getPosition());
				sn->setRotation(bc->getRotation());
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
				auto gc = e->getGraphicsComponent();
				if(auto sgc = dynamic_cast<SphereGraphicsComponent*>(gc.get()))
					sn = _smgr->addSphereSceneNode(sgc->getRadius(), 64, bsn, -1, gc->getPosOffset());
				sn->setName("graphics");
				sn->setDebugDataVisible(scene::EDS_FULL);
				break;
			}
		case ComponentType::GraphicsMesh:
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
				auto gc = e->getGraphicsComponent();
				if(auto sgc = dynamic_cast<MeshGraphicsComponent*>(gc.get()))
				{
					if(sgc->getFileName() == "")
						return;
					if(sgc->isAnimated())
						sn = _smgr->addAnimatedMeshSceneNode(_smgr->getMesh(("./media/" + sgc->getFileName()).c_str()), bsn, -1, gc->getPosOffset(), gc->getRotOffset(), gc->getScale());
					else
						sn = _smgr->addMeshSceneNode(_smgr->getMesh(("./media/" + sgc->getFileName()).c_str()), bsn, -1, gc->getPosOffset(), gc->getRotOffset(), gc->getScale());
					//sn = _smgr->addSphereSceneNode(1, 64, bsn, -1, gc->getPosOffset());
				}
				sn->setMaterialFlag(video::EMF_LIGHTING, false);
				sn->setName("graphics");
				//sn->setDebugDataVisible(scene::EDS_FULL);
				break;
			}
		default:
			break;
	}
}

void ViewSystem::onObservableRemove(EntityEvent&)
{}
