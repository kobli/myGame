#define _USE_MATH_DEFINES
#include "system.hpp"

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
		std::function<Entity*()> _getEntity;

	public:
		MyMotionState(std::function<Entity*()> getEntity): _getEntity{getEntity}
		{}

		virtual ~MyMotionState()
		{
		}

		virtual void getWorldTransform(btTransform& worldTrans) const
		{
			auto e = _getEntity();
			if(!e)
				return;
			auto bc = e->getComponent<BodyComponent>();
			if(!bc)
				return;
			auto cc = e->getComponent<CollisionComponent>();
			if(!cc)
				return;
			worldTrans.setOrigin(V3f2btV3f(bc->getPosition() - cc->getPosOffset()));
			//worldTrans.setRotation(Q2btQ(bc->getRotation()));
		}

		virtual void setWorldTransform(const btTransform& worldTrans)
		{
			auto e = _getEntity();
			if(!e)
				return;
			auto bc = e->getComponent<BodyComponent>();
			if(!bc)
				return;
			auto cc = e->getComponent<CollisionComponent>();
			if(!cc)
				return;
			/*
			btQuaternion rot = worldTrans.getRotation();
			vec3f r;
			btMatrix3x3(rot).getEulerZYX(r.Z, r.Y, r.X);
			bc->setRotation(r*180/M_PI);
			*/
			bc->setRotation(btQ2Q(worldTrans.getRotation()));
			//cout << "ROT: " << btQ2Q(worldTrans.getRotation()) << endl;
			bc->setPosition(btV3f2V3f(worldTrans.getOrigin()) + cc->getPosOffset());
		}
};

System::System(World& world): _world{world}
{
}

////////////////////////////////////////////////////////////

Physics::Physics(World& world, scene::ISceneManager* smgr): System{world}, _tAcc{0}, _updating{false}, _heightMap{nullptr}
{
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
		//_physicsWorld->getDebugDrawer()->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
		_physicsWorld->getDebugDrawer()->setDebugMode(btIDebugDraw::DBG_DrawText);
	}

	const WorldMap& m = _world.getMap();
	// physics setup for the terrain
	int upAxis = 1; // Y
	unsigned w = m.getSize().X;
	assert(m.getSize().X == m.getSize().Y);
	assert((w & (w - 1)) == 0); // map size must be power of two
	_heightMap.reset(new float[(w+1)*(w+1)]);
	float* heightMap = _heightMap.get();

	float min = std::numeric_limits<float>::max();
	float max = std::numeric_limits<float>::min();
	for(unsigned y = 0; y<w; y++)
	{
		for(unsigned x = 0; x<w; x++)
		{
			float h = m.getHeightAt(x,y);
			heightMap[x + y*(w)] = h;
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
	float f = 1;
	btVector3 localScaling(f, 1, f);
	terrS->setLocalScaling(localScaling);

	// set origin to middle of heightfield
	btTransform tr;
	tr.setIdentity();

	btVector3 aabbMin, aabbMax;
	terrS->getAabb(tr, aabbMin, aabbMax);
	tr.setOrigin(btVector3((aabbMax.getX()-aabbMin.getX())/2., (min+max)*localScaling.y()/2., (aabbMax.getZ()-aabbMin.getZ())/2.));

	// create ground object
	btDefaultMotionState* terrMS = new btDefaultMotionState(tr);
	btRigidBody::btRigidBodyConstructionInfo terrCI(0.0, terrMS, terrS);
	btRigidBody* terrB = new btRigidBody(terrCI);
	_physicsWorld->addRigidBody(terrB);
	terrB->setUserIndex(ObjStaticID::Map);
	terrB->setAnisotropicFriction(btVector3(0.8,0.3,0.8));
	//terrB->setFriction(1);
}

vec3f Physics::getObjVelocity(ID objID)
{
	auto co = getCollisionObjectByID(objID);
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
	_updating = true;
	auto unsetUpdating = std::unique_ptr<void, std::function<void(void*)>>(this, [this](void*) { _updating = false; });
	static unsigned fn = 0;
	float dt = 0.01;
	_tAcc += timeDelta;

	/*
	_physicsWorld->stepSimulation(timeDelta, 0.1/dt, dt);
	bodyDoStrafe(timeDelta);
	*/

	/*
	// horrible
	dt = 0.04;
	if(_tAcc < dt)
		return;
	timeDelta = _tAcc;
	_tAcc = 0;
	_physicsWorld->stepSimulation(timeDelta, 100);
	bodyDoStrafe(timeDelta);
	*/

	/*
	int steps = _tAcc/dt;
	timeDelta = steps*dt;
	_tAcc -= timeDelta;
	_physicsWorld->stepSimulation(timeDelta, steps, dt);
	bodyDoStrafe(timeDelta);
	*/

	timeDelta = 0;
	while(_tAcc >= dt)
	{
		_physicsWorld->stepSimulation(dt, 1, dt);
		bodyDoStrafe(dt);
		moveKinematics(dt);
		timeDelta += dt;
		_tAcc -= dt;
		fn++;
	}
	callCollisionCBs();
	_physicsWorld->debugDrawWorld();
}

void Physics::bodyDoStrafe(float timeDelta)
{
	for(auto& od : _objData)
		od.second.walkTimer += timeDelta;
	//TODO do this only for the bodies that collide with terrain (could be called from collision checking)
	for(auto& e: _world.getEntities())
	{
		auto bc = e.getComponent<BodyComponent>();
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
		
		//if(t > 0.35)
		{
			//t = 0;
			//float fMul = 19000;
			//float fMul = 13000;
			//float fMul = 4000;
			//float fMul = 1300;
			//float fMul = 4300;
			float fMul = 800;
			{
				vec2f strDir = bc->getStrafeDir();
				vec3f dir{strDir.X, 0, strDir.Y};
				vec3f rot;
				bc->getRotation().toEuler(rot);
				rot *= 180/PI;
				dir.rotateYZBy(-rot.X);
				dir.rotateXZBy(-rot.Y);
				dir.rotateXYBy(-rot.Z);
				dir.Y = 0;
				dir.normalize();

				//if(receiver.IsKeyDown(irr::KEY_SPACE))
					//dir.Y = 1;

				//dynamicsWorld->removeRigidBody(b);
				//pSphereShape->calculateLocalInertia(mass2, fallInertia);
				if(dir.getLength() > 0.1 && _objData[e.getID()].onGround)
				{
					b->setDamping(0.41, 0);
					if(getObjVelocity(e.getID()).getLength() < 2)
					{
						//cout << "climbing a hill\n";
						fMul *= 1.5;
					}
					b->setFriction(1.0);
					//b->setMassProps(mass, fallInertia);
					//b->applyCentralForce(btVector3(dir.X, dir.Y+0.1, dir.Z)*fMul);
					b->applyCentralImpulse(btVector3(dir.X, 0., dir.Z)*fMul*timeDelta);//*abs(cos(2*M_PI*(1/0.5)*t)));
					//b->applyCentralImpulse(btVector3(dir.X, 0.05, dir.Z)*fMul/*abs(cos(2*M_PI*(1/0.5)*t))*/); // !! working
					//t = 0;
				}
				else
				{
					t = 0;
					b->setFriction(10);
					//if(getObjVelocity(e.getID()).getLength() < 0.1)
						//b->setDamping(50, 0);
					
					//b->setMassProps(mass2, fallInertia);
				}
				//b->setLinearVelocity(b->getLinearVelocity() + btVector3(dir.X*MOVEMENT_SPEED, 0, dir.Z*MOVEMENT_SPEED));
				//dynamicsWorld->addRigidBody(b);
			}
		}
	}
}

void Physics::moveKinematics(float timeDelta)
{
	for(auto& e: _world.getEntities())
	{
		auto cc = e.getComponent<CollisionComponent>();
		auto bc = e.getComponent<BodyComponent>();
		if(bc && cc)
			if(cc->isKinematic())
			{
				vec3f p = bc->getPosition();
				p += bc->getVelocity()*timeDelta;
				bc->setPosition(p);
			}
	}

}

void Physics::callCollisionCBs()
{
	for(auto& d : _objData)
		d.second.onGround = false;
	int numManifolds = _physicsWorld->getDispatcher()->getNumManifolds();
	for (int i = 0; i < numManifolds; i++)
	{
		btPersistentManifold* contactManifold =  _physicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
		const btCollisionObject* obA = contactManifold->getBody0();
		const btCollisionObject* obB = contactManifold->getBody1();

		int numContacts = contactManifold->getNumContacts();
		if(numContacts > 0)
		{
			// set onGround flags
			int obj0ID = min(obA->getUserIndex(), obB->getUserIndex());
			int obj1ID = max(obA->getUserIndex(), obB->getUserIndex());
			if(obj0ID == ObjStaticID::Map)
				_objData[obj1ID].onGround = true;	
			for(auto& colCB : _collCallbacks)
				colCB(obj0ID, obj1ID);
		}
	}
}

void Physics::onMsg(const EntityEvent& m)
{
	switch(m.componentT)
	{
		case ComponentType::Collision:
			{
				auto eID = m.entityID;
				btCollisionObject* o = getCollisionObjectByID(eID);
				if(o)
					_physicsWorld->removeCollisionObject(o);
				if(m.destroyed)
					break;

				auto e = _world.getEntity(eID);
				if(!e)
					break;
				auto col = e->getComponent<CollisionComponent>();
				if(!col)
					break;
				btScalar mass = col->getMass();
				btScalar iner = 1;
				btVector3 fallInertia(iner, iner, iner);

				_objData.emplace(eID, ObjData{});
				btCollisionShape* pShape = new btCapsuleShape(col->getRadius(), col->getHeight());
				pShape->calculateLocalInertia(mass,fallInertia);
				MyMotionState* motionState = new MyMotionState([this, eID]()->Entity* { return _world.getEntity(eID); });
				btRigidBody::btRigidBodyConstructionInfo bodyCI(mass,motionState,pShape,fallInertia);
				btRigidBody* body = new btRigidBody(bodyCI);
				_physicsWorld->addRigidBody(body);
				body->setUserIndex(eID);
				//body->setCcdMotionThreshold(1e-7);
				//body->setCcdSweptSphereRadius(0.2);
				if(col->isKinematic())
				{
					body->setCollisionFlags(body->getCollisionFlags() |	btCollisionObject::CF_NO_CONTACT_RESPONSE | btCollisionObject::CF_KINEMATIC_OBJECT);
				}
				//body->setActivationState(DISABLE_DEACTIVATION);
				body->setAngularFactor(btVector3(0,0,0));
				body->setGravity(btVector3(0,col->getGravity(),0));
				//body->setRollingFriction(0);
				break;
			}
		case ComponentType::Body:
			{
				if(_updating)
					return;
				auto e = _world.getEntity(m.entityID);
				if(!e)
					break;
				auto cc = e->getComponent<CollisionComponent>();
				if(!cc)
					break;
				btCollisionObject* o = getCollisionObjectByID(m.entityID);
				if(!o)
					break;
				auto b = e->getComponent<BodyComponent>();
				if(!b)
					break;
				auto rigB = dynamic_cast<btRigidBody*>(o);
				auto v = b->getVelocity();
				//if(rigB) //TODO only for ghostObjects
					//rigB->setLinearVelocity(btVector3(v.X, v.Y, v.Z));
				float rotSpeed = 3;
				//_physicsWorld->removeCollisionObject(rigB);
				auto tr = btTransform(Q2btQ(b->getRotation()), V3f2btV3f(b->getPosition()-cc->getPosOffset()));
				//rigB->proceedToTransform(tr);
				//rigB->setInterpolationWorldTransform(tr);
				rigB->setWorldTransform(tr);
				//rigB->setCenterOfMassTransform(tr);
				//rigB->setLinearVelocity(btVector3(0,0,0));
				//rigB->clearForces();
				rigB->setAngularVelocity(btVector3(0, b->getRotDir()*rotSpeed, 0));
				//_physicsWorld->addCollisionObject(rigB);
				rigB->activate(true);
				break;
			}
		default:
			break;
	}
}

btCollisionObject* Physics::getCollisionObjectByID(ID entityID)
{
	for(int i = 0; i < _physicsWorld->getNumCollisionObjects(); i++)
	{
		auto obj = _physicsWorld->getCollisionObjectArray()[i];
		if(obj->getUserIndex() == entityID)
			return obj;
	}
	return nullptr;
}

void Physics::registerCollisionCallback(std::function<void(ID, ID)> callback)
{
	_collCallbacks.push_back(callback);
	_collCallbacks.push_back([callback](ID first, ID second) {
				callback(second, first);
			});
}

void Physics::registerPairCollisionCallback(std::function<void(ID, ID)> callback)
{
	_collCallbacks.push_back(callback);
}
		
////////////////////////////////////////////////////////////

ViewSystem::ViewSystem(irr::scene::ISceneManager* smgr, World& world): System{world}, _smgr{smgr}
{
}

void ViewSystem::onMsg(const EntityEvent& m)
{
	switch(m.componentT)
	{
		case ComponentType::Body:
			{
				scene::ISceneNode* sn = _smgr->getSceneNodeFromId(m.entityID);
				if(sn && m.destroyed)
				{
					sn->removeAll();// remove children
					sn->remove();
					return;
				}
				else if(!sn && !m.destroyed)
				{
					sn = _smgr->addEmptySceneNode(nullptr, m.entityID);
					sn->setName("body");
					//sn->setDebugDataVisible(scene::EDS_FULL);
				}
				_transformedEntities.insert(m.entityID);
				break;
			}
		case ComponentType::GraphicsSphere:
			{
				auto e = _world.getEntity(m.entityID);
				if(!e)
					return;
				if(!e->hasComponent(ComponentType::Body))
					return;
				auto bsn = _smgr->getSceneNodeFromId(m.entityID);
				if(!bsn)
					return;
				auto sn = _smgr->getSceneNodeFromName("graphicsSphere", bsn);
				if(sn)
					sn->remove();
				if(m.destroyed)
					return;
				if(auto sgc = e->getComponent<SphereGraphicsComponent>()) {
					sn = _smgr->addSphereSceneNode(sgc->getRadius(), 64, bsn, -1, sgc->getPosOffset());
					sn->setMaterialFlag(video::EMF_LIGHTING, false);
					sn->setMaterialFlag(video::EMF_WIREFRAME, true);
					sn->setName("graphicsSphere");
				}
				break;
			}
		case ComponentType::GraphicsParticleSystem:
			{
				auto e = _world.getEntity(m.entityID);
				if(!e)
					return;
				if(!e->hasComponent(ComponentType::Body))
					return;
				auto bsn = _smgr->getSceneNodeFromId(m.entityID);
				if(!bsn)
					return;
				auto sn = static_cast<scene::IParticleSystemSceneNode*>(_smgr->getSceneNodeFromName("graphicsParticleSystem", bsn));
				if(sn)
					sn->remove();
				if(m.destroyed)
					return;
				if(auto psgc = e->getComponent<ParticleSystemGraphicsComponent>()) {
					sn = _smgr->addParticleSystemSceneNode(true, bsn, -1, psgc->getPosOffset(), psgc->getRotOffset(), psgc->getScale());
					addParticleEffect(psgc->getEffectID(), sn);
					sn->setName("graphicsParticleSystem");
				}
				break;
			}
		case ComponentType::GraphicsMesh:
			{
				auto e = _world.getEntity(m.entityID);
				if(!e)
					return;
				if(!e->hasComponent(ComponentType::Body))
					return;
				auto bsn = _smgr->getSceneNodeFromId(m.entityID);
				if(!bsn)
					return;
				auto sn = _smgr->getSceneNodeFromName("graphicsMesh", bsn);
				if(sn)
					sn->remove();
				if(m.destroyed)
					return;
				if(auto mgc = e->getComponent<MeshGraphicsComponent>())
				{
					if(mgc->getFileName() == "")
						return;
					if(mgc->isAnimated())
						sn = _smgr->addAnimatedMeshSceneNode(_smgr->getMesh(("./media/" + mgc->getFileName()).c_str()), bsn, -1, mgc->getPosOffset(), mgc->getRotOffset(), mgc->getScale());
					else
						sn = _smgr->addMeshSceneNode(_smgr->getMesh(("./media/" + mgc->getFileName()).c_str()), bsn, -1, mgc->getPosOffset(), mgc->getRotOffset(), mgc->getScale());
					sn->setMaterialFlag(video::EMF_LIGHTING, false);
					sn->setName("graphicsMesh");
					//sn->setDebugDataVisible(scene::EDS_FULL);
				}
				break;
			}
		default:
			break;
	}
}

void ViewSystem::update(float timeDelta)
{
	updateTransforms(timeDelta);
}

void ViewSystem::updateTransforms(float timeDelta)
{
	//TODO remove the list?
	for(auto it = _transformedEntities.begin(); it != _transformedEntities.end(); ) {
		auto eID = *it;
		Entity* e;
		BodyComponent* bc;
		scene::ISceneNode* sn;
		if((e = _world.getEntity(eID)) && (bc = e->getComponent<BodyComponent>()) && (sn = _smgr->getSceneNodeFromId(eID)))
		{
			quaternion newRot = bc->getRotation();
			static const float rotationInterpolationSpeed = 4;
			quaternion resRot = newRot;
			resRot.slerp(quaternion(sn->getRotation()/180*PI), newRot, std::min(1.f,rotationInterpolationSpeed*timeDelta));
			vec3f r;
			resRot.toEuler(r);
			sn->setRotation(r*180/PI);

			vec3f oldPos = sn->getPosition();
			vec3f newPos = bc->getPosition();
			vec3f resPos;
			vec3f d = newPos - oldPos;
			float snapThreshold = 5;
			float interpolSpeed = d.getLength()*8;
			if(d.getLength() > snapThreshold)
				resPos = newPos;
			else if(d.getLength() < interpolSpeed*timeDelta)
				resPos = oldPos;
			else
				resPos = oldPos + d.normalize()*interpolSpeed*timeDelta;
			//resPos = newPos; //TODO remove
			/*
			std::cout << "obj desired p: " << resPos << std::endl;
			std::cout << "obj abs p: " << _smgr->getSceneNodeFromId(0)->getAbsolutePosition() << std::endl;
			std::cout << "obj p: " << _smgr->getSceneNodeFromId(0)->getPosition() << std::endl;
			std::cout << "cam abs p: " << _smgr->getSceneNodeFromId(-2)->getAbsolutePosition() << std::endl;
			std::cout << "cam p: " << _smgr->getSceneNodeFromId(ObjStaticID::Camera)->getPosition() << std::endl;
			*/
			sn->setPosition(resPos);
			sn->updateAbsolutePosition();
			if(resPos != newPos || resRot != newRot) {
				it++;
				continue;
			}
		}
		_transformedEntities.erase(it++);
	}
}

scene::IParticleEmitter* ViewSystem::addParticleEffect(ID effectID, scene::IParticleSystemSceneNode* sn)
{
	scene::IParticleEmitter* em = nullptr;
	scene::IParticleAffector* paff = nullptr;
	std::string effectTextureName;
	vec3f particleDir;
	video::SColor partColorMin = video::SColor(255,0,0,0);
	video::SColor partColorMax = video::SColor(255,255,255,255);
	core::dimension2df particleSize(.2,.2);
	u32 particleC = 700;


	switch(effectID) {
		case 0: // empty body
			{
				effectTextureName = "body";
				break;
			}
		case 1: // fire
			{
				effectTextureName = "fire";
				particleDir = vec3f(0,0.001,0);
				break;
			}
		case 2: // water
			{
				effectTextureName = "water";
				break;
			}
		case 3: // heal
			{
				effectTextureName = "heal";
				break;
			}
		default:
			return nullptr;
	}
	paff = sn->createFadeOutParticleAffector();

	std::string texturePath = "./media/"+effectTextureName+".bmp";
	em = sn->createSphereEmitter(
			vec3f(0),
			0.6,
			particleDir,
			particleC,particleC*1.3,
			partColorMin,
			partColorMax,
			800,1000,10,
			particleSize,
			particleSize*1.5);
	if(paff != nullptr) {
		sn->addAffector(paff);
		paff->drop();
	}
	sn->setMaterialFlag(video::EMF_LIGHTING, false);
	sn->setMaterialFlag(video::EMF_ZWRITE_ENABLE, false);
	sn->setMaterialTexture(0, _smgr->getVideoDriver()->getTexture(texturePath.c_str()));
	sn->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
	if(em) {
		sn->setEmitter(em);
		em->drop();
	}
}

////////////////////////////////////////////////////////////

SpellSystem::SpellSystem(World& world): System{world}, _luaState{nullptr}
{
	init();
}

SpellSystem::~SpellSystem()
{
	deinit();
}

void SpellSystem::update(float timeDelta)
{
	lUpdate(timeDelta);
	for(auto& e : _world.getEntities())
		if(e.hasComponent<WizardComponent>())
			if(e.hasComponent<BodyComponent>())
				lReportWalkingWizard(e.getID(), e.getComponent<BodyComponent>()->getStrafeDir() != vec2f(0,0));
}

void SpellSystem::onMsg(const EntityEvent& m)
{
	if(m.componentT == ComponentType::Wizard)
	{
		if(m.created)
		{

#ifdef DEBUG_BUILD
			reload(); // for more comfortable testing - every time a player connects reset and reload the spellsystem
#endif
			addWizard(m.entityID);
		}
		else if(m.destroyed)
			removeWizard(m.entityID);
	}
}

void SpellSystem::reload()
{
	deinit();
	init();
}

void SpellSystem::addWizard(ID entID)
{
	if(_luaState != nullptr)
	{
		lua_getglobal(_luaState, "addWizard");
		lua_pushinteger(_luaState, entID);
		if(lua_pcall(_luaState, 1, 0, 0) != 0)
		{
			cerr << "something went wrong with addWizard: " << lua_tostring(_luaState, -1) << endl;
			lua_pop(_luaState, 1);
		}
	}
}

void SpellSystem::removeWizard(ID entID)
{
	if(_luaState != nullptr)
	{
		lua_getglobal(_luaState, "removeWizard");
		lua_pushinteger(_luaState, entID);
		if(lua_pcall(_luaState, 1, 0, 0) != 0)
		{
			cerr << "something went wrong with removeWizard: " << lua_tostring(_luaState, -1) << endl;
			lua_pop(_luaState, 1);
		}
	}
}

void SpellSystem::cast(std::string& incantation, ID authorID)
{
	lua_getglobal(_luaState, "handleIncantation");
	lua_pushinteger(_luaState, authorID);
	lua_pushstring(_luaState, incantation.c_str());
	if(lua_pcall(_luaState, 2, 0, 0) != 0)
	{
		cerr << "something went wrong with handleIncantation: " << lua_tostring(_luaState, -1) << endl;
		lua_pop(_luaState, 1);
	}
}

void SpellSystem::collisionCallback(ID objID, ID otherObjID)
{
	//std::cout << "WIZ COMP: collision of " << objID << " and " << otherObjID << std::endl;
	lua_getglobal(_luaState, "handleCollision");
	lua_pushinteger(_luaState, objID);
	lua_pushinteger(_luaState, otherObjID);
	if(lua_pcall(_luaState, 2, 0, 0) != 0)
	{
		cerr << "something went wrong with handleCollision: " << lua_tostring(_luaState, -1) << endl;
		lua_pop(_luaState, 1);
	}
}

void SpellSystem::lUpdate(float timeDelta)
{
	if(_luaState != nullptr)
	{
		lua_getglobal(_luaState, "update");
		lua_pushnumber(_luaState, timeDelta);
		if(lua_pcall(_luaState, 1, 0, 0) != 0)
		{
			cerr << "something went wrong with spell update: " << lua_tostring(_luaState, -1) << endl;
			lua_pop(_luaState, 1);
		}
	}
}

void SpellSystem::lReportWalkingWizard(ID wizID, bool walking)
{
	if(_luaState != nullptr)
	{
		lua_getglobal(_luaState, "wizardWalking");
		lua_pushinteger(_luaState, wizID);
		lua_pushboolean(_luaState, walking);
		if(lua_pcall(_luaState, 2, 0, 0) != 0)
		{
			cerr << "something went wrong with wizardWalking: " << lua_tostring(_luaState, -1) << endl;
			lua_pop(_luaState, 1);
		}
	}
}

void SpellSystem::init()
{
	_luaState = luaL_newstate();
	luaL_openlibs(_luaState);
	luaL_dofile(_luaState, "lua/spellSystem.lua");

	auto callLaunchSpell = [](lua_State* s)->int {
		int argc = lua_gettop(s);
		if(argc != 5)
		{
			std::cerr << "callLaunchSpell: wrong number of arguments\n";
			return 0;		
		}
		ID wizard = lua_tointeger(s, 1);
		float sRadius = lua_tonumber(s, 2);
		float sSpeed = lua_tonumber(s, 3);
		float sElevation = lua_tonumber(s, 4);
		ID sEffectID = lua_tointeger(s, 5);
		SpellSystem* ss = (SpellSystem*)lua_touserdata(s, lua_upvalueindex(1));
		ID spell = ss->launchSpell(sRadius, sSpeed, sElevation, wizard, sEffectID);
		lua_pushinteger(s, spell);
		return 1;
	};
	lua_pushlightuserdata(_luaState, this);
	lua_pushcclosure(_luaState, callLaunchSpell, 1);
	lua_setglobal(_luaState, "wizardLaunchSpell");

	auto callRemoveSpell = [](lua_State* s)->int {
		int argc = lua_gettop(s);
		if(argc != 1)
		{
			std::cerr << "callRemoveSpell: wrong number of arguments\n";
			return 0;		
		}
		ID spell = lua_tointeger(s, 1);
		SpellSystem* ss = (SpellSystem*)lua_touserdata(s, lua_upvalueindex(1));
		ss->removeSpell(spell);
		return 1;
	};
	lua_pushlightuserdata(_luaState, this);
	lua_pushcclosure(_luaState, callRemoveSpell, 1);
	lua_setglobal(_luaState, "removeSpell");

	auto callAddAttributeAffector = [](lua_State* s)->int {
		int argc = lua_gettop(s);
		if(argc != 5 && argc != 6)
		{
			std::cerr << "callAddAttributeAffector: wrong number of arguments\n";
			return 0;		
		}
		ID eID = lua_tointeger(s, 1);
		std::string attributeName = lua_tostring(s, 2);
		AttributeAffector::ModifierType modifierType = static_cast<AttributeAffector::ModifierType>(lua_tointeger(s, 3));
		float modifierValue = lua_tonumber(s, 4);
		bool permanent = lua_toboolean(s, 5);
		float period = 0;
		if(argc == 6)
			period = lua_tonumber(s, 6);
		SpellSystem* ss = (SpellSystem*)lua_touserdata(s, lua_upvalueindex(1));
		ss->addAttributeAffectorTo(eID, attributeName, modifierType, modifierValue, permanent, period);
		return 1;
	};
	lua_pushlightuserdata(_luaState, this);
	lua_pushcclosure(_luaState, callAddAttributeAffector, 1);
	lua_setglobal(_luaState, "addAttributeAffector");

	auto callSetEntityVelocity = [](lua_State* s)->int {
		int argc = lua_gettop(s);
		if(argc != 4)
		{
			std::cerr << "callSetEntityVelocity: wrong number of arguments\n";
			return 0;		
		}
		ID eID = lua_tointeger(s, 1);
		vec3f velocity;
		velocity.X = lua_tonumber(s, 2);
		velocity.Y = lua_tonumber(s, 3);
		velocity.Z = lua_tonumber(s, 4);
		World* world = (World*)lua_touserdata(s, lua_upvalueindex(1));
		Entity* e = world->getEntity(eID);
		if(e != nullptr) {
			BodyComponent* bc = e->getComponent<BodyComponent>();
			if(bc != nullptr)
				bc->setVelocity(velocity);
		}
		return 0;
	};
	lua_pushlightuserdata(_luaState, &_world);
	lua_pushcclosure(_luaState, callSetEntityVelocity, 1);
	lua_setglobal(_luaState, "setEntityVelocity");

	auto updateWizardStatus = [](lua_State* s)->int {
		int argc = lua_gettop(s);
		if(argc != 7)
		{
			std::cerr << "updateWizardStatus: wrong number of arguments\n";
			return 0;		
		}
		ID eID = lua_tointeger(s, 1);
		std::string currentJob = lua_tostring(s, 2);
		float currentJobDuration = lua_tonumber(s, 3);
		float currentJobProgress = lua_tonumber(s, 4);
		float spellInHandsPower = lua_tonumber(s, 5);
		float spellInHandsRadius = lua_tonumber(s, 6);
		float spellInHandsSpeed = lua_tonumber(s, 7);
		World* world = (World*)lua_touserdata(s, lua_upvalueindex(1));
		Entity* e = world->getEntity(eID);
		if(e != nullptr) {
			WizardComponent* wc = e->getComponent<WizardComponent>();
			if(wc != nullptr) {
				wc->setCurrentJobStatus(currentJob, currentJobDuration, currentJobProgress);
				wc->setSpellInHandsData(spellInHandsPower, spellInHandsRadius, spellInHandsSpeed);
			}
		}
		return 0;
	};
	lua_pushlightuserdata(_luaState, &_world);
	lua_pushcclosure(_luaState, updateWizardStatus, 1);
	lua_setglobal(_luaState, "updateWizardStatus");
}

void SpellSystem::deinit()
{
	lua_close(_luaState);
}

ID SpellSystem::launchSpell(float radius, float speed, float elevation, ID wizard, ID spellEffectID)
{
	auto e = _world.getEntity(wizard);
	if(!e)
		return NULLID; //TODO fail in a better way
	auto wBody = e->getComponent<BodyComponent>();
	if(!wBody)
		return NULLID; //TODO fail in a better way

	Entity& spellE = _world.createAndGetEntity();
	vec3f rot;
	wBody->getRotation().toEuler(rot);
	vec3f dir = ((rot/PI*180)+vec3f(-elevation,0,0)).rotationToDirection().normalize();
	dir.rotateXZBy(-90);
	vec3f pos = wBody->getPosition() + vec3f(0,1.5,0) + dir*(radius + 0.6);
	spellE.addComponent<BodyComponent>(pos, quaternion(), dir*speed);
	spellE.addComponent<CollisionComponent>(radius, 0, vec3f(0), 1, true, 0);
#ifdef DEBUG_BUILD
	spellE.addComponent<SphereGraphicsComponent>(radius);
#endif
	spellE.addComponent<ParticleSystemGraphicsComponent>(spellEffectID, vec3f(0), vec3f(0), vec3f(radius));

	return spellE.getID();
}

void SpellSystem::removeSpell(ID spell)
{
	std::cout << "removing spell #" << spell << std::endl;
	_world.removeEntity(spell);
}

ID SpellSystem::addAttributeAffectorTo(ID eID, std::string attributeName
		, AttributeAffector::ModifierType modifierType, float modifierValue
		, bool permanent, float period)
{
	auto e = _world.getEntity(eID);
	if(!e)
		return NULLID; //TODO fail in a better way
	auto attrStore = e->getComponent<AttributeStoreComponent>();
	if(!attrStore)
		return NULLID; //TODO fail in a better way
	std::cout << "adding attribute affector .." << std::endl;
	return attrStore->addAttributeAffector(AttributeAffector(attributeName, modifierType, modifierValue, permanent, period));
}

////////////////////////////////////////////////////////////

InputSystem::InputSystem(World& world, SpellSystem& spells): System{world}, _spells{spells}
{
}

void InputSystem::handleCommand(Command& c, ID controlledObjID)
{
	switch(c._type)
	{
		case Command::Type::STRAFE_DIR_SET:
			{
				auto bc = getBodyComponent(controlledObjID);
				if(bc)
					bc->setStrafeDir(c._vec2f);
				break;
			}
		case Command::Type::ROT_DIR_SET:
			{
				auto bc = getBodyComponent(controlledObjID);
				if(bc)
					;//bc->setRotDir(c._i32);
				break;
			}
		case Command::Type::STR:
			{
				if(c._str.find("spell_") == 0)
				{
					_spells.cast(c._str, controlledObjID);
				}
				break;
			}
		case Command::Type::Y_ANGLE_SET:
			{
				auto bc = getBodyComponent(controlledObjID);
				if(bc) {
					quaternion q = bc->getRotation();
					vec3f e;
					q.toEuler(e);
					q = /*quaternion(e.X,0,e.Z)*/quaternion(0,c._float,0); // TODO retain previous X and Z rotation
					bc->setRotation(q);
				}
				break;
			}
		default:
			cerr << "unknown command type to handle\n";
	}
}

BodyComponent* InputSystem::getBodyComponent(ID objID)
{
	auto e = _world.getEntity(objID);
	if(e)
		return e->getComponent<BodyComponent>();
	else
		return nullptr;
}
