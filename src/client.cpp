#include <cassert>
#include "client.hpp"
#include "network.hpp"
#include "serdes.hpp"
Animator::Animator(scene::ISceneManager* smgr, function<Entity*(u32)> entityResolver, function<vec3f(u32)> entityVelocityGetter)
	: _smgr{smgr}, _entityResolver{entityResolver}, _velGetter{entityVelocityGetter}
{}

void Animator::setEntityResolver(std::function<Entity*(u32 ID)> entityResolver)
{
	_entityResolver = entityResolver;
}

void Animator::setEntityVelocityGetter(std::function<vec3f(u32 ID)> entityVelocityGetter)
{
	_velGetter = entityVelocityGetter;
}

void Animator::setSceneManager(scene::ISceneManager* smgr)
{
	_smgr = smgr;
}

void Animator::onMsg(const EntityEvent& m)
{
	if(m.componentT != ComponentType::Body)
		return;
	Entity* ePtr = _entityResolver(m.entityID);
	if(!ePtr)
		return;
	Entity& e = *ePtr;
	auto b = e.getComponent<BodyComponent>();
	if(!b)
		return;
	auto* mgc = e.getComponent<MeshGraphicsComponent>();
	if(!mgc || !mgc->isAnimated())
		return;
	// play idle anim
	int firstFrame = 190;
	int lastFrame = 290;
	float animSpeed = 5;
	float speed = _velGetter(m.entityID).getLength();
	{
		if(b->getStrafeDir().X == 1) {
			// play walk forward anim
			firstFrame = 0;
			lastFrame = 13;
			animSpeed *= speed;
		}
		else if(b->getStrafeDir().X == -1) {
			// play walk forward anim with reverse speed (speed 15 at strafeSpeed 3)
			firstFrame = 0;
			lastFrame = 13;
			animSpeed *= -speed;
		}
	}
	if(!_smgr)
		return;
	scene::ISceneNode* bsn = _smgr->getSceneNodeFromId(m.entityID);
	if(!bsn)
		return;
	scene::ISceneNode* sn = _smgr->getSceneNodeFromName("graphics", bsn);
	if(!sn)
		return;
	if(sn->getType() != scene::ESCENE_NODE_TYPE::ESNT_ANIMATED_MESH)
		return;
	scene::IAnimatedMeshSceneNode* asn = static_cast<scene::IAnimatedMeshSceneNode*>(sn);
	if(!asn)
		return;
	if(firstFrame != asn->getStartFrame() || lastFrame != asn->getEndFrame())
		asn->setFrameLoop(firstFrame, lastFrame);
	asn->setAnimationSpeed(animSpeed);
}

////////////////////////////////////////////////////////////

ClientApplication::ClientApplication(): _device(nullptr, [](IrrlichtDevice* d){ if(d) d->drop(); })
{
	_server.setBlocking(false);

	irr::SIrrlichtCreationParameters params;
	params.DriverType=video::E_DRIVER_TYPE::EDT_OPENGL;
	params.WindowSize=core::dimension2d<u32>(640, 480);
	_device.reset(createDeviceEx(params));
	if(_device == 0)
		cerr << "Failed to create Irrlicht device\n";
	
	_device->setEventReceiver(&_controller);

	_device->getVideoDriver()->setTextureCreationFlag(video::ETCF_ALWAYS_32_BIT, true);

	_device->getCursorControl()->setVisible(false);

	createWorld();
	createCamera();

	_controller.setCommandHandler(std::bind(&ClientApplication::commandHandler, ref(*this), std::placeholders::_1));
	_controller.setScreenSizeGetter([this](){ auto ss = _device->getVideoDriver()->getScreenSize(); return vec2i(ss.Width, ss.Height); });
}

bool ClientApplication::connect(string host, short port)
{
	auto r = _server.connect(host, port);
	return (r != sf::Socket::Error && r != sf::Socket::Disconnected);
}

void ClientApplication::run()
{
	int lastFPS = -1;
	auto driver = _device->getVideoDriver();

	sf::Clock c;
	while(_device->run())
	{
		if(_camera->isInputReceiverEnabled())
			bindCameraToControlledEntity();
		if(!_camera->isInputReceiverEnabled()) {
			vec3f cameraLookDir((_cameraElevation-M_PI_2)/M_PI*180,(_cameraYAngle+M_PI_2)/M_PI*180,0);
			cameraLookDir = cameraLookDir.rotationToDirection().normalize();
			_camera->setTarget(_camera->getAbsolutePosition()+cameraLookDir*10000);
		}

		while(receive());		
		//TODO fix frameLen spike after win inactivity (mind the physics)
		if(true)//if(_device->isWindowActive())
		{
			if(_device->isWindowActive())
				_device->getCursorControl()->setPosition(vec2f(0.5));
			driver->beginScene();
			f32 ar = (float)driver->getScreenSize().Width/(float)driver->getScreenSize().Height;
			if(ar != _camera->getAspectRatio() && _camera)
				_camera->setAspectRatio(ar);
			float timeDelta = c.restart().asSeconds();
			//std::cout << "number of scene nodes: " << _device->getSceneManager()->getRootSceneNode()->getChildren().size() << std::endl;
				
			if(_physics)
				_physics->update(timeDelta);
			if(_vs)
				_vs->update(timeDelta);

			_device->getSceneManager()->drawAll();
			//_device->getGUIEnvironment()->drawAll();
			driver->endScene();

			// display frames per second in window title
			int fps = driver->getFPS();
			if (lastFPS != fps)
			{
				core::stringw str = L"Terrain Renderer - Irrlicht Engine [";
				str += driver->getName();
				str += "] FPS:";
				str += fps;

				_device->setWindowCaption(str.c_str());
				lastFPS = fps;
			}
		}
		sf::sleep(sf::milliseconds(1));
	}
}

void ClientApplication::createWorld()
{
	_worldMap.reset(new WorldMap(70, _device->getSceneManager()));
	_gameWorld.reset(new World(*_worldMap));
	_vs.reset(new ViewSystem(_device->getSceneManager(), *_gameWorld));
	_physics.reset(new Physics(*_gameWorld, _device->getSceneManager()));
	_animator.setEntityResolver(bind(&World::getEntity, ref(*_gameWorld), placeholders::_1));
	_animator.setSceneManager(_device->getSceneManager());
	_animator.setEntityVelocityGetter([this](u32 ID)->vec3f {	return _physics->getObjVelocity(ID); });
	_gameWorld->addObserver(_animator);
	_gameWorld->addObserver(*_physics);
	_gameWorld->addObserver(*_vs);
}

void ClientApplication::createCamera()
{
	SKeyMap keyMap[9];
	keyMap[0].Action = EKA_MOVE_FORWARD;
	keyMap[0].KeyCode = KEY_UP;
	keyMap[1].Action = EKA_MOVE_FORWARD;
	keyMap[1].KeyCode = KEY_KEY_W;
	keyMap[2].Action = EKA_MOVE_BACKWARD;
	keyMap[2].KeyCode = KEY_DOWN;
	keyMap[3].Action = EKA_MOVE_BACKWARD;
	keyMap[3].KeyCode = KEY_KEY_S;
	keyMap[4].Action = EKA_STRAFE_LEFT;
	keyMap[4].KeyCode = KEY_LEFT;
	keyMap[5].Action = EKA_STRAFE_LEFT;
	keyMap[5].KeyCode = KEY_KEY_A;
	keyMap[6].Action = EKA_STRAFE_RIGHT;
	keyMap[6].KeyCode = KEY_RIGHT;
	keyMap[7].Action = EKA_STRAFE_RIGHT;
	keyMap[7].KeyCode = KEY_KEY_D;
	keyMap[8].Action = EKA_JUMP_UP;
	keyMap[8].KeyCode = KEY_SPACE;
	f32 camWalkSpeed = 0.05f;
	_camera = 
		_device->getSceneManager()->addCameraSceneNodeFPS(nullptr,100.0f,camWalkSpeed,ObjStaticID::Camera,keyMap,9,false);
	
	_camera->setPosition(core::vector3df(0,10,50));
	//_camera->setTarget(core::vector3df(2397*2,343*2,2700*2));
	_camera->setFarValue(42000.0f);

	if(!_worldMap)
		return;
	// create collision response animator and attach it to the camera
	
//	scene::ISceneNodeAnimatorCollisionResponse* anim = _device->getSceneManager()->createCollisionResponseAnimator(
//		_worldMap->getMetaTriangleSelector(), _camera, core::vector3df(40,100,40),
//		core::vector3df(0,-50,0),
//		core::vector3df(0,50,0),
//		0.1);
//	_camera->addAnimator(anim);
}

void ClientApplication::commandHandler(Command& c)
{
	if(c._type == Command::Type::ROT_diff && !_camera->isInputReceiverEnabled()) {
		_cameraYAngle += c._vec2f.X;
		_cameraElevation = std::min(PI-0.2f, std::max(0.3f, _cameraElevation+c._vec2f.Y));
	}
	sendCommand(c);
}

void ClientApplication::sendCommand(Command& c)
{
	sf::Packet p;
	p << PacketType::PlayerCommand << c;
	sendPacket(p);
}

void ClientApplication::sendPacket(sf::Packet& p)
{
	sf::Socket::Status r;
	while((r = _server.send(p)) == sf::Socket::Status::Partial);
	//TODO handle disconnect and errors
	if(r == sf::Socket::Status::NotReady)
		cerr << "Socket is not ready to send data.\n";
	else if(r == sf::Socket::Status::Error)
		cerr << "An error occured while sending a packet.\n";
	else if(r == sf::Socket::Status::Disconnected)
		cerr << "Server disconnected.\n";
}

bool ClientApplication::receive()
{
	sf::Packet p;		
	sf::Socket::Status r;
	while((r = _server.receive(p)) == sf::Socket::Status::Partial);
	//TODO handle disconnect and errrors
	if(r == sf::Socket::Status::Done)
	{
		handlePacket(p);
		return true;
	}
	else if(r == sf::Socket::Status::NotReady)
		;//cerr << "Socket not ready to receive.\n"; // no data to receive
	else if(r == sf::Socket::Status::Error)
		cerr << "An error occured while receiving data.\n";
	else if(r == sf::Socket::Status::Disconnected)
		cerr << "Server disconnected.\n";
	return false;
}

void ClientApplication::handlePacket(sf::Packet& p)
{
	PacketType t;
	p >> t;
	switch(t)
	{
		case PacketType::WorldUpdate:
			{
				EntityEvent e(NULLID);	
				p >> e;
				handleEntityEvent(e);
				auto* entity = _gameWorld->getEntity(e.entityID);
				ObservableComponentBase* modifiedComponent = nullptr;
				if(entity && (modifiedComponent = entity->getComponent(e.componentT)))
				{
					p >> Deserializer<sf::Packet>(*modifiedComponent);
					modifiedComponent->notifyObservers();
					//cout << "updated component: " << Serializer<ostream>(*modifiedComponent) << endl;
				}
				if(e.created)
					cout << "CREATED!\n";
				if(e.destroyed && e.componentT == ComponentType::NONE) // TODO respond to component deletes
				{
					cout << "SHOULD DELETE ENTITY\n";
					_gameWorld->removeEntity(e.entityID);
				}
				break;
			}
		case PacketType::RegistryUpdate:
			{
					p >> Deserializer<sf::Packet>(_sharedRegistry);
					cout << "shared reg update: " << Serializer<ostream>(_sharedRegistry) << endl;
					onSharedRegistryUpdated();
				break;
			}
		default:
			cerr << "Received packet of unknown type.\n";
	}
}

void ClientApplication::handleEntityEvent(EntityEvent& e)
{
	//cout << "received entity update: " << e.entityID << " " << e.componentT << endl;
	if(!_gameWorld)
		return;
	Entity* entity = _gameWorld->getEntity(e.entityID);
	if(!entity) {
		entity = &_gameWorld->createAndGetEntity(e.entityID);
		std::cout << "HAD TO CREATE NEW ENTITY\n";
	}
	if(entity->getID() != e.entityID)
		cerr << "ENTITY IDs DO NOT MATCH - requested " << e.entityID << " - got " << entity->getID() << "\n";
	//TODO no need to create wizard component
	entity->addComponent(e.componentT);
}

void ClientApplication::onSharedRegistryUpdated()
{
}

void ClientApplication::bindCameraToControlledEntity()
{
	if(_sharedRegistry.hasKey("controlled_object_id")) {
		auto controlledCharSceneNode = _device->getSceneManager()->getSceneNodeFromId(_sharedRegistry.getValue("controlled_object_id"));
		if(controlledCharSceneNode) {
			_camera->setInputReceiverEnabled(false);
			_camera->setParent(controlledCharSceneNode);
			_camera->setPosition(vec3f(0.23,1.6,0));
			_camera->setRotation(vec3f());
			_camera->bindTargetAndRotation(false);
			_cameraElevation = PI_2;
			_cameraYAngle = 0;
		}
	}
}
