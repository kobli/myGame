#include <cassert>
#include "client.hpp"
#include "network.hpp"
#include "serdes.hpp"
#include "heightmapMesh.hpp"
#include "terrainTexturer.hpp"

class MyShaderCallBack : public video::IShaderConstantSetCallBack
{
	public:
		MyShaderCallBack(irr::IrrlichtDevice* device): _device{device}
		{}

		virtual void OnSetConstants(video::IMaterialRendererServices* services, s32)
		{
			video::IVideoDriver* driver = services->getVideoDriver();

			core::matrix4 worldViewProj;
			worldViewProj *= driver->getTransform(video::ETS_PROJECTION);
			worldViewProj *= driver->getTransform(video::ETS_VIEW);
			worldViewProj *= driver->getTransform(video::ETS_WORLD);
			services->setVertexShaderConstant("mWorldViewProj", worldViewProj.pointer(), 16);

			s32 TextureLayerID[] = {0, 1, 2, 3}; 
			services->setPixelShaderConstant("textures[0]", TextureLayerID, 4);
		}
	private:
		irr::IrrlichtDevice* _device;
};

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

ClientApplication::ClientApplication(): _device(nullptr, [](IrrlichtDevice* d){ if(d) d->drop(); }),
	_yAngleSetCommandFilter{0.1, [](float& oldObj, float& newObj)->float&{ if(std::fabs(oldObj-newObj) > 0.1) return newObj; else return oldObj; }},
	_healthBar{nullptr}
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

	createWorld();
	createCamera();

	io::path psFileName = "./media/opengl.frag";
	io::path vsFileName = "./media/opengl.vert";

	auto driver = _device->getVideoDriver();
	if (!driver->queryFeature(video::EVDF_PIXEL_SHADER_1_1) &&
			!driver->queryFeature(video::EVDF_ARB_FRAGMENT_PROGRAM_1))
	{
		_device->getLogger()->log("WARNING: Pixel shaders disabled "\
				"because of missing driver/hardware support.");
		psFileName = "";
	}

	if (!driver->queryFeature(video::EVDF_VERTEX_SHADER_1_1) &&
			!driver->queryFeature(video::EVDF_ARB_VERTEX_PROGRAM_1))
	{
		_device->getLogger()->log("WARNING: Vertex shaders disabled "\
				"because of missing driver/hardware support.");
		vsFileName = "";
	}

	// create material
	video::IGPUProgrammingServices* gpu = driver->getGPUProgrammingServices();
	s32 multiTextureMaterialType = 0;

	if (gpu)
	{
		MyShaderCallBack* mc = new MyShaderCallBack(_device.get());

		multiTextureMaterialType = gpu->addHighLevelShaderMaterialFromFiles(
				vsFileName, "vertexMain", video::EVST_VS_1_1,
				psFileName, "pixelMain", video::EPST_PS_1_1,
				mc, video::EMT_TRANSPARENT_VERTEX_ALPHA, 0 , video::EGSL_DEFAULT);
		mc->drop();
	}


	HeightmapMesh mesh;
	mesh.init(_worldMap->getTerrain(), /*[](f32,f32,f32,vec3f){return video::SColor(255,255,0,0);}*/TerrainTexturer::texture, _device->getVideoDriver());
	scene::IMeshSceneNode* terrain = _device->getSceneManager()->addMeshSceneNode(mesh.Mesh, nullptr);
	terrain->setPosition(terrain->getBoundingBox().getCenter()*core::vector3df(-1,0,-1));


	terrain->setMaterialFlag(video::EMF_BACK_FACE_CULLING, false);
	terrain->setMaterialFlag(video::EMF_LIGHTING, false);
	//terrain->setMaterialFlag(video::EMF_WIREFRAME, true);
	terrain->setMaterialFlag(video::EMF_BLEND_OPERATION, true);
	terrain->setMaterialType((video::E_MATERIAL_TYPE)multiTextureMaterialType);
	terrain->setMaterialTexture(TerrainTexture::grass, driver->getTexture("./media/grass.jpg"));
	terrain->setMaterialTexture(TerrainTexture::rock, driver->getTexture("./media/rock.jpg"));
	terrain->setMaterialTexture(TerrainTexture::snow, driver->getTexture("./media/snow.jpg"));
	terrain->setMaterialTexture(TerrainTexture::sand, driver->getTexture("./media/sand.jpg"));
	terrain->getMaterial(0).TextureLayer->getTextureMatrix().setTextureScale(10,10);
	terrain->getMaterial(0).TextureLayer->TextureWrapU = video::E_TEXTURE_CLAMP::ETC_REPEAT;
	terrain->getMaterial(0).TextureLayer->TextureWrapV = video::E_TEXTURE_CLAMP::ETC_REPEAT;


	auto screenSize = _device->getVideoDriver()->getScreenSize();
	gui::IGUIEnvironment* env = _device->getGUIEnvironment();

	auto* skin = env->getSkin();
	auto* font = env->getFont("./media/fontlucida.png");
	if(!font)
		std::cerr << "FAILED TO LOAD THE FONT\n";
	skin->setFont(font);

	_healthBar = new gui::ProgressBar(env, core::rect<s32>(20, 20, 220, 60), env->getRootGUIElement());
	_healthBar->setColors(video::SColor(155, 255,255,255), video::SColor(200, 255,0,0));
	env->addStaticText(L"", core::rect<s32>(0, 0, _healthBar->getRelativePosition().getWidth(), _healthBar->getRelativePosition().getHeight()), false, false, _healthBar);
	_healthBar->drop();

	int castIndLen = 200;
	_castingIndicator = new gui::ProgressBar(env, core::rect<s32>(0, 0, castIndLen, 40), env->getRootGUIElement());
	_castingIndicator->setRelativePosition(vec2i((screenSize.Width-castIndLen)/2, 30));
	_castingIndicator->setAlignment(gui::EGUI_ALIGNMENT::EGUIA_CENTER, gui::EGUI_ALIGNMENT::EGUIA_CENTER, gui::EGUI_ALIGNMENT::EGUIA_UPPERLEFT, gui::EGUI_ALIGNMENT::EGUIA_UPPERLEFT);
	_castingIndicator->setColors(video::SColor(155, 255,255,255), video::SColor(200, 0,0,255));
	env->addStaticText(L"", core::rect<s32>(0, 0, _castingIndicator->getRelativePosition().getWidth(), _castingIndicator->getRelativePosition().getHeight()), false, false, _castingIndicator);
	_castingIndicator->drop();

	int spellInHandsInfoPanelWidth = 100;
	_spellInHandsInfo = _device->getGUIEnvironment()->addStaticText(L"SIH info", core::rect<s32>(0, 0, spellInHandsInfoPanelWidth, 80), false, false, env->getRootGUIElement(), -1, true);
	_spellInHandsInfo->setBackgroundColor(video::SColor(200, 255,255,255));
	_spellInHandsInfo->setRelativePosition(vec2i(screenSize.Width-spellInHandsInfoPanelWidth-30, 30));
	_spellInHandsInfo->setAlignment(gui::EGUI_ALIGNMENT::EGUIA_LOWERRIGHT, gui::EGUI_ALIGNMENT::EGUIA_LOWERRIGHT, gui::EGUI_ALIGNMENT::EGUIA_UPPERLEFT, gui::EGUI_ALIGNMENT::EGUIA_UPPERLEFT);

	_controller.setCommandHandler(std::bind(&ClientApplication::commandHandler, ref(*this), std::placeholders::_1));
	_controller.setScreenSizeGetter([this](){ auto ss = _device->getVideoDriver()->getScreenSize(); return vec2i(ss.Width, ss.Height); });
	_controller.setExit([this](){ _device->closeDevice(); });
}

bool ClientApplication::connect(string host, unsigned short port)
{
	std::cout << "Connecting to " << host << ":" << port << std::endl;
	auto r = _server.connect(host, port);
	_server.setBlocking(false);
	return r == sf::Socket::Done;
}

void ClientApplication::run()
{
	int lastFPS = -1;
	auto driver = _device->getVideoDriver();

	sf::Clock c;
	while(_device->run())
	{
		if(_camera->isInputReceiverEnabled() && !_controller.isCameraFree())
			bindCameraToControlledEntity();
		if(!_camera->isInputReceiverEnabled()) {
			vec3f cameraLookDir((_cameraElevation-PI_2)/PI*180,(_cameraYAngle+PI_2)/PI*180,0);
			cameraLookDir = cameraLookDir.rotationToDirection().normalize();
			_camera->setTarget(_camera->getAbsolutePosition()+cameraLookDir*10000);
			if(_sharedRegistry.hasKey("controlled_object_id")) {
				auto controlledCharSceneNode = _device->getSceneManager()->getSceneNodeFromId(_sharedRegistry.getValue("controlled_object_id"));
				if(controlledCharSceneNode)
					_camera->setPosition(controlledCharSceneNode->getPosition() + vec3f(0,1.6,0) + 0.23f*(cameraLookDir*vec3f(1,0,1)).normalize());
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
			if(ar != _camera->getAspectRatio() && _camera)
				_camera->setAspectRatio(ar);
			float timeDelta = c.restart().asSeconds();
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
			updateCastingIndicator(timeDelta);

			_device->getSceneManager()->drawAll();
			_device->getGUIEnvironment()->drawAll();
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
	_worldMap.reset(new WorldMap());
	_worldMap->generate(vec2u(128),1);
	_gameWorld.reset(new World(*_worldMap));
	_vs.reset(new ViewSystem(_device->getSceneManager(), *_gameWorld));
	_physics.reset(new Physics(*_gameWorld, _device->getSceneManager()));
	_animator.setEntityResolver(bind(&World::getEntity, ref(*_gameWorld), placeholders::_1));
	_animator.setSceneManager(_device->getSceneManager());
	_animator.setEntityVelocityGetter([this](u32 ID)->vec3f {	return _physics->getObjVelocity(ID); });
	_gameWorld->addObserver(_animator);
	_gameWorld->addObserver(*_physics);
	_gameWorld->addObserver(*_vs);
	_gameWorld->addObserver(*this);
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
	_camera->setTarget(core::vector3df(0));
	_camera->setFarValue(42000.0f);
}

void ClientApplication::commandHandler(Command& c)
{
	if(c._type == Command::Type::ROT_diff && !_camera->isInputReceiverEnabled()) {
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
		default:
			cerr << "Received packet of unknown type.\n";
	}
}

void ClientApplication::bindCameraToControlledEntity()
{
	if(_sharedRegistry.hasKey("controlled_object_id")) {
		auto controlledCharSceneNode = _device->getSceneManager()->getSceneNodeFromId(_sharedRegistry.getValue("controlled_object_id"));
		if(controlledCharSceneNode) {
			_camera->setInputReceiverEnabled(false);
			_camera->setRotation(vec3f());
			_camera->bindTargetAndRotation(false);
			_cameraElevation = PI_2;
			_cameraYAngle = 0;
		}
	}
}

void ClientApplication::onMsg(const EntityEvent& m)
{
	Entity* controlledE = nullptr;
	if(_sharedRegistry.hasKey("controlled_object_id")) {
		ID id = _sharedRegistry.getValue("controlled_object_id");
		controlledE = _gameWorld->getEntity(id);
	}
	if(m.componentT == ComponentType::AttributeStore) {
		_healthBar->setProgress(0);
		if(controlledE != nullptr) {
			AttributeStoreComponent* as = controlledE->getComponent<AttributeStoreComponent>();
			if(as != nullptr) {
				if(as->hasAttribute("health") && as->hasAttribute("max-health")) {
					_healthBar->setProgress(as->getAttribute("health")/as->getAttribute("max-health"));
					std::wstring w = std::to_wstring(int(as->getAttribute("health"))) + L" / " + std::to_wstring(int(as->getAttribute("max-health")));
					auto text = static_cast<gui::IGUIStaticText*>(*_healthBar->getChildren().begin());
					text->setText(w.c_str());
					vec2i hbSize(_healthBar->getAbsolutePosition().getWidth(), _healthBar->getAbsolutePosition().getHeight());
					text->setRelativePosition((hbSize-vec2i(text->getTextWidth(), text->getTextHeight()))/2);
					return;
				}
			}
		}
	}
	else if(m.componentT == ComponentType::Wizard)
		if(controlledE != nullptr) {
			WizardComponent* wc = controlledE->getComponent<WizardComponent>();
			if(wc != nullptr) {
				_spellInHandsInfo->setText(std::wstring(
						L"Power:   "+std::to_wstring(wc->getSpellInHandsPower()) + L"\n" +
						L"Radius:  "+std::to_wstring(wc->getSpellInHandsRadius()) + L"\n" +
						L"Speed:   "+std::to_wstring(wc->getSpellInHandsSpeed())
						).c_str());
				if(!wc->getCurrentJob().empty()) {
					_castingIndicator->setVisible(true);
					_castingIndicator->setProgress(wc->getCurrentJobProgress()/wc->getCurrentJobDuration());
					std::string job = wc->getCurrentJob();
					std::wstring w;
					w.assign(job.begin(), job.end());
					auto text = static_cast<gui::IGUIStaticText*>(*_castingIndicator->getChildren().begin());
					text->setText(w.c_str());
					vec2i ciSize(_castingIndicator->getAbsolutePosition().getWidth(), _castingIndicator->getAbsolutePosition().getHeight());
					text->setRelativePosition((ciSize-vec2i(text->getTextWidth(), text->getTextHeight()))/2);
				}
				else
					_castingIndicator->setVisible(false);
			}
		}
}

void ClientApplication::updateCastingIndicator(float timeDelta)
{
	if(_sharedRegistry.hasKey("controlled_object_id")) {
		ID id = _sharedRegistry.getValue("controlled_object_id");
		Entity* e = _gameWorld->getEntity(id);
		if(e != nullptr) {
			WizardComponent* wc = e->getComponent<WizardComponent>();
			if(wc != nullptr)
				_castingIndicator->setProgress(_castingIndicator->getProgress() + timeDelta/wc->getCurrentJobDuration());
		}
	}
}
