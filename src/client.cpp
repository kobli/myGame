#include <cassert>
#include "client.hpp"
#include "network.hpp"
#include "serdes.hpp"

Animator::Animator(scene::ISceneManager* smgr, function<Entity*(ID)> entityResolver, function<vec3f(ID)> entityVelocityGetter)
	: _smgr{smgr}, _entityResolver{entityResolver}, _velGetter{entityVelocityGetter}
{}

void Animator::setEntityResolver(std::function<Entity*(ID)> entityResolver)
{
	_entityResolver = entityResolver;
}

void Animator::setEntityVelocityGetter(std::function<vec3f(ID)> entityVelocityGetter)
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
	scene::ISceneNode* sn = _smgr->getSceneNodeFromName("graphicsMesh", bsn);
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

ClientApplication::ClientApplication(): _device(nullptr, [](IrrlichtDevice* d){ if(d) d->drop(); }), _controller{nullptr},
	_yAngleSetCommandFilter{0.2, [](float& oldObj, float& newObj)->float&{ if(std::fabs(oldObj-newObj) > 0.01) return newObj; else return oldObj; }}
{
	irr::SIrrlichtCreationParameters params;
	params.DriverType=video::E_DRIVER_TYPE::EDT_OPENGL;
	params.WindowSize=core::dimension2d<u32>(640, 480);
	_device.reset(createDeviceEx(params));
	if(_device == 0)
		cerr << "Failed to create Irrlicht device\n";
	
	_device->setEventReceiver(&_controller);

	_device->getVideoDriver()->setTextureCreationFlag(video::ETCF_ALWAYS_32_BIT, true);

	_device->getCursorControl()->setVisible(false);
	_device->setResizable(true);
	
	SAVEIMAGE = ImageDumper(_device->getVideoDriver());

	_controller.setCommandHandler(std::bind(&ClientApplication::commandHandler, ref(*this), std::placeholders::_1));
	_controller.setScreenSizeGetter([this](){ auto ss = _device->getVideoDriver()->getScreenSize(); return vec2i(ss.Width, ss.Height); });
	_controller.setExit([this](){ _device->closeDevice(); });
	_controller.setDevice(_device.get());

	_device->getSceneManager()->addSkyDomeSceneNode(_device->getVideoDriver()->getTexture("media/skydome.jpg"), 16,8,0.95f,2.0f,1000, nullptr, ObjStaticID::Skybox);
}

bool ClientApplication::connect(string host, unsigned short port)
{
	std::cout << "Connecting to " << host << ":" << port << std::endl;
	auto r = _server.connect(host, port);
	_server.setBlocking(false);
	sendHello();
	return r == sf::Socket::Done;
}

vec3f interpolate(vec3f current, vec3f target, float timeDelta, float maxSpeed, float maxDistance)
{
	float distance = current.getDistanceFrom(target);
	if(distance > maxDistance)
		return target;
	else {
		float d = std::min(distance, (distance/maxDistance)*maxSpeed*timeDelta);
		return current + (target-current).normalize()*d;
	}
}

void ClientApplication::run()
{
	int lastFPS = -1;
	auto driver = _device->getVideoDriver();

	sf::Clock c;
	while(_device->run())
	{
		float timeDelta = c.restart().asSeconds();
		scene::ICameraSceneNode* camera = getCamera();
		if(camera) {
			if(camera->isInputReceiverEnabled() && !_controller.isCameraFree())
				bindCameraToControlledEntity();
			if(!camera->isInputReceiverEnabled()) {
				vec3f cameraLookDir((_cameraElevation-PI_2)/PI*180,(_cameraYAngle+PI_2)/PI*180,0);
				cameraLookDir = cameraLookDir.rotationToDirection().normalize();
				camera->setTarget(camera->getAbsolutePosition()+cameraLookDir*10000);
				if(_sharedRegistry.hasKey("controlled_object_id")) {
					auto controlledCharSceneNode = _device->getSceneManager()->getSceneNodeFromId(_sharedRegistry.getValue<ID>("controlled_object_id"));
					if(controlledCharSceneNode) {
						controlledCharSceneNode->setVisible(false);
						camera->setPosition(interpolate(
									camera->getPosition(),
									controlledCharSceneNode->getPosition() + vec3f(0,1.6,0) + 0.23f*(cameraLookDir*vec3f(1,0,1)).normalize(),
									timeDelta,
									10,
									1
									));
					}
				}
			}
		}

		while(receive());		
		//TODO fix frameLen spike after win inactivity (mind the physics)
		if(true)//if(_device->isWindowActive())
		{
			if(_device->isWindowActive())
				_device->getCursorControl()->setPosition(vec2f(0.5));
			driver->beginScene(/*true,true,video::SColor(255,255,255,255)*/);
			f32 ar = (float)driver->getScreenSize().Width/(float)driver->getScreenSize().Height;
			camera = getCamera();
			if(camera && ar != camera->getAspectRatio())
				camera->setAspectRatio(ar);
			//std::cout << "number of scene nodes: " << _device->getSceneManager()->getRootSceneNode()->getChildren().size() << std::endl;
				
			if(_yAngleSetCommandFilter.tick(timeDelta) && _yAngleSetCommandFilter.objUpdated()) {
				Command c(Command::Type::Y_ANGLE_SET);
				c._float = _yAngleSetCommandFilter.reset();
				sendCommand(c);
			}

			if(_physics)
				_physics->update(timeDelta);
			if(_vs)
				_vs->update(timeDelta);
			if(_gui)
				_gui->update(timeDelta);

			_device->getSceneManager()->drawAll();
			_device->getGUIEnvironment()->drawAll();

			driver->runAllOcclusionQueries(false);
			driver->updateAllOcclusionQueries();

			driver->endScene();

			// display frames per second in window title
			int fps = driver->getFPS();
			if (lastFPS != fps)
			{
				core::stringw str = L"MyGame [";
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

void ClientApplication::startGame()
{
	std::cout << "GAME STARTING\n";
	_gameWorld.reset(new World(*_worldMap));
	_vs.reset();
	_vs.reset(new ViewSystem(_device->getSceneManager(), *_gameWorld));
	_physics.reset(new Physics(*_gameWorld, _device->getSceneManager()));
	_animator.setEntityResolver(bind(&World::getEntity, ref(*_gameWorld), placeholders::_1));
	_animator.setSceneManager(_device->getSceneManager());
	_animator.setEntityVelocityGetter([this](ID id)->vec3f {	return _physics->getObjVelocity(id); });
	_gameWorld->addObserver(_animator);
	_gameWorld->addObserver(*_physics);
	_gameWorld->addObserver(*_vs);
	_gui.reset();
	_gui.reset(new GUI(_device.get(), *_gameWorld.get(), _sharedRegistry, _gameRegistry));
	_gameWorld->addObserver(*_gui);
	createCamera();

	if(_controller.getSettings().hasKey("NAME")) {
		Command c;
		c._type = Command::Type::STR;
		c._str = "NAME "+_controller.getSettings().getValue<std::string>("NAME");
		sendCommand(c);
	}
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
	auto camera = 
		_device->getSceneManager()->addCameraSceneNodeFPS(nullptr,100.0f,camWalkSpeed,ObjStaticID::Camera,keyMap,9,false);
	
	camera->setPosition(core::vector3df(0,10,50));
	camera->setTarget(core::vector3df(0));
	camera->setFarValue(42000.0f);
}

void ClientApplication::commandHandler(Command& c)
{
	if(c._type == Command::Type::ROT_diff && getCamera() && !getCamera()->isInputReceiverEnabled()) {
		_cameraYAngle += c._vec2f.X;
		_cameraYAngle = std::fmod(_cameraYAngle, PI*2);
		_cameraElevation = std::min(PI-0.2f, std::max(0.3f, _cameraElevation+c._vec2f.Y));

		_yAngleSetCommandFilter.filter(_cameraYAngle);
		//Command nc(Command::Type::Y_ANGLE_SET);
		//nc._float = _cameraYAngle;
		//sendCommand(nc);
	}
	else if(c._type == Command::Type::STR) {
		size_t kp = c._str.find("$LOOK_ELEVATION");
		if(kp != std::string::npos)
			c._str.replace(kp, strlen("$LOOK_ELEVATION"), std::to_string(int(-(_cameraElevation/PI*180)+90)));
		sendCommand(c);
	}
	else
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
				if(!_gameWorld)
					return;
				EntityEvent event(NULLID);	
				p >> event;

				Entity* entity = nullptr;
				if(event.created && event.componentT == ComponentType::NONE) {
					// this may happen: when new client connects, server broadcasts create events for all objects
					//assert(_gameWorld->getEntity(event.entityID) == nullptr);
					if((entity = _gameWorld->getEntity(event.entityID)) == nullptr)
						entity = &_gameWorld->createAndGetEntity(event.entityID);
					if(entity->getID() != event.entityID)
						cerr << "ENTITY IDs DO NOT MATCH - requested " << event.entityID << " - got " << entity->getID() << "\n";
				}
				else if(event.destroyed && event.componentT == ComponentType::NONE) {
					_gameWorld->removeEntity(event.entityID);
				}
				else if((entity = _gameWorld->getEntity(event.entityID)) != nullptr) {
					ObservableComponentBase* modifiedComponent = nullptr;
					if(event.created)
						entity->addComponent(event.componentT);
					else if(event.destroyed)
						entity->removeComponent(event.componentT);
					if((modifiedComponent = entity->getComponent(event.componentT)) != nullptr) {
						p >> Deserializer<sf::Packet>(*modifiedComponent);
						//std::cout << Serializer<std::ostream>(*modifiedComponent) << std::endl;
						modifiedComponent->notifyObservers();
					}
				}
				break;
			}
		case PacketType::RegistryUpdate:
			{
					p >> Deserializer<sf::Packet>(_sharedRegistry);
					cout << "shared reg update: " << Serializer<ostream>(_sharedRegistry) << endl;
				break;
			}
		case PacketType::GameRegistryUpdate:
			{
					p >> Deserializer<sf::Packet>(_gameRegistry);
					cout << "game reg update: " << Serializer<ostream>(_gameRegistry) << endl;
				break;
			}
		case PacketType::GameInit:
			{
				_worldMap.reset(new WorldMap());
				p >> Deserializer<sf::Packet>(*_worldMap);
				startGame();
				break;
			}
		case PacketType::Message:
			{
				string message;
				p >> message;
				displayMessage(message);
				break;
			}
			case PacketType::GameOver:
			{
				sf::sleep(sf::seconds(1));
				sf::Packet p;
				p << PacketType::JoinGame;
				sendPacket(p);
				break;
			}
		default:
			cerr << "Received packet of unknown type.\n";
	}
}

void ClientApplication::bindCameraToControlledEntity()
{
	if(_sharedRegistry.hasKey("controlled_object_id")) {
		auto controlledCharSceneNode = _device->getSceneManager()->getSceneNodeFromId(_sharedRegistry.getValue<ID>("controlled_object_id"));
		auto camera = getCamera();
		if(controlledCharSceneNode) {
			camera->setInputReceiverEnabled(false);
			camera->setRotation(vec3f());
			camera->bindTargetAndRotation(false);
			_cameraElevation = PI_2;
			_cameraYAngle = 0;
		}
	}
}

void ClientApplication::sendHello()
{
	sf::Packet p;
	p << PacketType::ClientHello << u16(myGame_VERSION_MAJOR) << u16(myGame_VERSION_MINOR);
	sendPacket(p);
}

void ClientApplication::displayMessage(std::string message)
{
	std::string tag;
	if(message.find("{") == 0) {
		int end = message.find("}");
		tag = message.substr(1, end-1);
		message = message.substr(end+1);
	}
	if(_gui)
		_gui->displayMessage(message, tag);
	//else
		std::cout << "Message" << (tag.length()?std::string(" (")+tag+std::string(")"):std::string())	<< ": " << message << std::endl;
}

scene::ICameraSceneNode* ClientApplication::getCamera()
{
	return static_cast<scene::ICameraSceneNode*>(_device->getSceneManager()->getSceneNodeFromId(ObjStaticID::Camera));
}
