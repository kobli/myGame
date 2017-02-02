#include <client.hpp>
#include <cassert>
#include <network.hpp>
#include <serdes.hpp>

Animator::Animator(function<WorldEntity*(u32)> entityResolver): _entityResolver{entityResolver}
{}

void Animator::setEntityResolver(std::function<WorldEntity*(u32 ID)> entityResolver)
{
	_entityResolver = entityResolver;
}

void Animator::onObservableAdd(EntityEvent& m)
{
	onObservableUpdate(m);
}

void Animator::onObservableUpdate(EntityEvent& m)
{
	WorldEntity* ePtr = _entityResolver(m._entityID);
	if(!ePtr)
		return;
	WorldEntity& e = *ePtr;
	//TODO if(e.getGraphicsComponent() == nullptr || !e.getGraphicsComponent()->isAnimated())
		return;
	// play idle anim
	unsigned firstFrame = 190;
	unsigned lastFrame = 290;
	float animSpeed = 1;
	if(m._componentModifiedType == ComponentType::Body)
	{
		auto b = e.getBodyComponent();
		assert(b != nullptr);
		if(b->getStrafeDir().X == 1) {
			// play walk forward anim
			firstFrame = 0;
			lastFrame = 13;
			animSpeed *= b->getStrafeSpeed()/10;
		}
		else if(b->getStrafeDir().X == -1) {
			// play walk forward anim with reverse speed (speed 15 at strafeSpeed 3)
			firstFrame = 0;
			lastFrame = 13;
			animSpeed *= -b->getStrafeSpeed()/10;
		}
	}
	scene::IAnimatedMeshSceneNode* n = nullptr;//TODO static_cast<scene::IAnimatedMeshSceneNode*>(e.getGraphicsComponent()->getSceneNode());
	n->setFrameLoop(firstFrame, lastFrame);
	n->setAnimationSpeed(animSpeed);
}

void Animator::onObservableRemove(EntityEvent&)
{
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

	_controller.setCommandHandler(std::bind(&ClientApplication::sendCommand, ref(*this), std::placeholders::_1));
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

	while(_device->run())
	{
		while(receive());		
		//TODO fix frameLen spike after win inactivity
		if(true)//if(_device->isWindowActive())
		{
			driver->beginScene(true, true, 0 );
			f32 ar = (float)driver->getScreenSize().Width/(float)driver->getScreenSize().Height;
			if(ar && _camera)
				_camera->setAspectRatio(ar);
			float timeDelta = 1./lastFPS;

			//if(_gameWorld)
				//_gameWorld->update(1./lastFPS);
				
			if(_physics)
				_physics->update(timeDelta);


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
	}
}

void ClientApplication::createWorld()
{
	_worldMap.reset(new WorldMap(200, _device->getSceneManager()));
	_gameWorld.reset(new World(*_worldMap));
	_vs.reset(new ViewSystem(_device->getSceneManager(), *_gameWorld));
	_physics.reset(new Physics(*_gameWorld, _device->getSceneManager()));
	_animator.setEntityResolver(bind(&World::getEntityByID, ref(*_gameWorld), placeholders::_1));
	_animator.observe(*_gameWorld);
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
	f32 camWalkSpeed = 0.2f;
	_camera = 
		_device->getSceneManager()->addCameraSceneNodeFPS(0,100.0f,camWalkSpeed,0,keyMap,9,false);
	
	_camera->setPosition(core::vector3df(0,100,0));
	//_camera->setTarget(core::vector3df(2397*2,343*2,2700*2));
	_camera->setFarValue(42000.0f);

	if(!_worldMap)
		return;
	// create collision response animator and attach it to the camera
	/*
	scene::ISceneNodeAnimatorCollisionResponse* anim = _device->getSceneManager()->createCollisionResponseAnimator(
		_worldMap->getMetaTriangleSelector(), _camera, core::vector3df(40,100,40),
		core::vector3df(0,-50,0),
		core::vector3df(0,50,0),
		0.1);
	_camera->addAnimator(anim);
	*/
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
	cout << "received packet\n";
	PacketType t;
	p >> t;
	switch(t)
	{
		case PacketType::WorldUpdate:
			{
				EntityEvent e;	
				p >> e;
				handleEntityEvent(e);
				if(e._componentModified != nullptr)
				{
					p >> Deserializer<sf::Packet>(*e._componentModified);
					e._componentModified->notifyObservers();
					cout << "updated component: " << Serializer<ostream>(*e._componentModified) << endl;
					if(e._created)
						cout << "CREATED!\n";
					if(e._destroyed)
						cout << "DESTROYED!\n";
				}
				break;
			}
		default:
			cerr << "Received packet of unknown type.\n";
	}
}

void ClientApplication::handleEntityEvent(EntityEvent& e)
{
	cout << "received entity update: " << e._entityID << " " << e._componentModifiedType << endl;
	if(!_gameWorld)
		return;
	WorldEntity* entity = _gameWorld->getEntityByID(e._entityID);
	if(!entity)
		entity = &_gameWorld->createEntity(e._entityID);
	if(entity->getID() != e._entityID)
		cerr << "ENTITY IDs DO NOT MATCH\n";
	switch(e._componentModifiedType)
	{
		case ComponentType::Collision: // TODO collider radius not synced, gravity not synced
			e._componentModified = entity->getCollisionComponent().get();
			if(!e._componentModified)
				e._componentModified = entity->setCollisionComponent(make_shared<CollisionComponent>(*entity)).get();
			break;
		case ComponentType::Input: // TODO connect inputCompopnent with controller for client side prediction
			e._componentModified = entity->getInputComponent().get();
			if(!e._componentModified)
				e._componentModified = entity->setInputComponent(make_shared<InputComponent>(*entity)).get();
			break;
		case ComponentType::Body:
			e._componentModified = entity->getBodyComponent().get();
			if(!e._componentModified)
				e._componentModified = entity->setBodyComponent(make_shared<BodyComponent>(*entity)).get();
			break;
		case ComponentType::GraphicsSphere: // TODO sync _animated (and ...?)
			e._componentModified = entity->getGraphicsComponent().get();
			if(!e._componentModified)
			{
				//scene::IAnimatedMeshSceneNode* sceneNode = _device->getSceneManager()->addAnimatedMeshSceneNode(_device->getSceneManager()->getMesh("./media/ninja.b3d"));
				e._componentModified = entity->setGraphicsComponent(make_shared<SphereGraphicsComponent>(*entity)).get();
			}
			break;
		case ComponentType::Wizard:
			cout << "someone has become a wizard\n";
			break;
		case ComponentType::None:
			cout << "update of component of type None\n";
			break;
		default:
			cerr << "Received update of unknown component type.\n";
			break;
	}
}
