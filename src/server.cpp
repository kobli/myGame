#include <server.hpp>
#include <cassert>
#include <serdes.hpp>

Session::Session(unique_ptr<sf::TcpSocket>&& socket, CommandHandler h)
	: _socket{std::move(socket)}, _commandHandler{h}, _closed{false}, _authorized{false}
{
	this->addObserver(*this);
	addPair("controlled_object_id", NULLID);
}

sf::TcpSocket& Session::getSocket()
{
	return *_socket;
}

void Session::setCommandHandler(CommandHandler h)
{
	_commandHandler = h;
}

ID Session::getControlledObjID()
{
	return getValue<ID>("controlled_object_id");
}

void Session::setControlledObjID(ID objID)
{
	setValue("controlled_object_id", objID);
}

bool Session::receive()
{
	sf::Packet p;
	sf::Socket::Status r = _socket->receive(p);
	if(r == sf::Socket::Status::Disconnected)
	{
		_closed = true;
		return false;
	}
	else if(r == sf::Socket::Status::Error)
	{
		cerr << "An error occured while receiving packet.\n";
		_closed = true;
		return false;
	}
	else if(r == sf::Socket::Status::NotReady)
	{
	 	// this just means that there is no data available
		return false;
	}
	handlePacket(p);
	return true;
}

void Session::send(sf::Packet& p)
{
	sf::Socket::Status r;
	while((r = _socket->send(p)) == sf::Socket::Status::Partial);
	if(r == sf::Socket::Status::Disconnected)
		_closed = true;
	else if(r == sf::Socket::Status::Error)
	{
		cerr << "An error occured while sending packet.\n";
		_closed = true;
	}
	else if(r == sf::Socket::Status::NotReady)
		cerr << "Attept to send data through a socket that was not ready.\n";
}

void Session::handlePacket(sf::Packet& p)
{
	PacketType pt;
	p >> pt;
	switch(pt)
	{
		case PacketType::PlayerCommand:
		{
			disconnectUnauthorized();
			Command c;
			p >> c;
			//cout << "command type: " << unsigned(c._type) << endl;
			_commandHandler(c, getControlledObjID());
			break;
		}
		case PacketType::ClientHello:
		{
			u16 vMajor, vMinor;
			p >> vMajor >> vMinor;
			if(vMajor != u16(myGame_VERSION_MAJOR) || vMinor != u16(myGame_VERSION_MINOR))
				disconnectUnauthorized("Version mismatch.");
			else {
				_authorized = true;
				_onAuthorized();
			}
			break;
		}
		default:
			cerr << "Received unknown packet type.\n";
	}
}

bool Session::isClosed()
{
	return _closed;
}

void Session::setOnAuthorized(OnAuthorized cb)
{
	_onAuthorized = cb;
}

void Session::addPair(std::string key, float value)
{
	Store::addPair(key, value);
}

template <typename T>
void Session::setValue(std::string key, T value)
{
	Store::setValue(key, value);
}

void Session::onMsg(const MessageT& m)
{
	sf::Packet p;
	p << m.typeID << Serializer<sf::Packet>(*static_cast<KeyValueStore*>(m.store));
	send(p);
}

void Session::disconnectUnauthorized(std::string reason)
{
	if(!_authorized)
	{
		std::cout << "Disconnecting unauthorized client: " << reason << std::endl;
		sf::Packet p;
		p << PacketType::ServerMessage << reason;
		send(p);
		_socket->disconnect();
	}
}

////////////////////////////////////////////////////////////

Updater::Updater(Sender s, EntityResolver getEntity): _send{s}, _getEntity{getEntity}, _timeSinceLastUpdateSent{0}
{}

void Updater::tick(float delta)
{
	_timeSinceLastUpdateSent += delta;
	if(_timeSinceLastUpdateSent >= 0.2) {
		_timeSinceLastUpdateSent = 0;
		while(!_updateEventQueue.empty()) {
			sendEvent(*_updateEventQueue.begin());
			_updateEventQueue.erase(_updateEventQueue.begin());
		}
	}
}

void Updater::onMsg(const EntityEvent& e)
{
	if(e.created || e.destroyed)
		sendEvent(e);
	else
		_updateEventQueue.insert(e);
}

void Updater::sendEvent(const EntityEvent& e)
{
	ObservableComponentBase* modifiedComponent = nullptr;
	auto* entity = _getEntity(e.entityID);
	if(entity)
		modifiedComponent = entity->getComponent(e.componentT);
	sf::Packet p;
	p << PacketType::WorldUpdate << e;
	if(modifiedComponent && !e.destroyed)
		p << Serializer<sf::Packet>(*modifiedComponent);
	_send(p, [](ID){ return true; });

	/*
	cout << "sent an update:\n\tentityID: " << e.entityID 
		<< "\n\tcomponent modified type: " << e.componentT << endl;
	if(modifiedComponent != nullptr && !e.destroyed)
		cout << "\tcomponent: " << Serializer<ostream>(*modifiedComponent) << endl;
		*/
}

////////////////////////////////////////////////////////////

Game::Game(const WorldMap& map): _map{map}, _gameWorld{_map}, _physics{_gameWorld}, _spells{_gameWorld}, _input{_gameWorld, _spells}, _LuaStateGameMode{nullptr},
	_gameModeEntityEventObserver{[this](const EntityEvent& e){ this->gameModeOnEntityEvent(e); }}, _ended{false}
{
	_gameWorld.addObserver(*this);
	_physics.registerCollisionCallback(std::bind(&SpellSystem::collisionCallback, std::ref(_spells), placeholders::_1, placeholders::_2));

	_LuaStateGameMode = luaL_newstate();
	luaL_openlibs(_LuaStateGameMode);
	gameModeRegisterAPIMethods();
	if(luaL_dofile(_LuaStateGameMode, "lua/gamemode_dm.lua"))
		printf("%s\n", lua_tostring(_LuaStateGameMode, -1));

	loadMap();
	gameModeOnGameStart();
}

Game::~Game()
{
	lua_close(_LuaStateGameMode);
}

bool Game::run(float timeDelta)
{
	while(!_eventQueue.empty()) {
		EntityEvent e = _eventQueue.front();
		_eventQueue.pop();
		_spells.onMsg(e);
		_physics.onMsg(e);
		_gameModeEntityEventObserver.onMsg(e);
	}

	_physics.update(timeDelta);
	_spells.update(timeDelta);

	return !_ended;
}

void Game::onMessage(const EntityEvent& m)
{
	_eventQueue.push(m);
	Observabler::onMessage(m);
}


void Game::loadMap() 
{
	for(const Tree& t: _map.getTrees()) {
		Entity& te = _gameWorld.createAndGetEntity();
		te.addComponent<BodyComponent>(t.position);
		te.addComponent<MeshGraphicsComponent>("Tree1.obj", false);
		te.addComponent<CollisionComponent>(0.5, 10, vec3f(0,-5.5,0), 0);
	}
	for(const Spawnpoint& s: _map.getSpawnpoints()) {
		Entity& te = _gameWorld.createAndGetEntity();
		te.addComponent<BodyComponent>(s.position);
		te.addComponent<AttributeStoreComponent>();
		te.getComponent<AttributeStoreComponent>()->addAttribute("spawnpoint",0);
	}
}

void Game::gameModeRegisterAPIMethods()
{
	lua_State* L = _LuaStateGameMode;
	assert(L != nullptr);
	lua_pushinteger(L, NULLID);
	lua_setglobal(L, "NULLID");

	lua_newtable(L);
	lua_pushliteral(L, "NONE"); lua_pushinteger(L, ComponentType::NONE); lua_settable(L, -3);
	lua_pushliteral(L, "Body"); lua_pushinteger(L, ComponentType::Body); lua_settable(L, -3);
	lua_pushliteral(L, "GraphicsSphere"); lua_pushinteger(L, ComponentType::GraphicsSphere); lua_settable(L, -3);
	lua_pushliteral(L, "GraphicsMesh"); lua_pushinteger(L, ComponentType::GraphicsMesh); lua_settable(L, -3);
	lua_pushliteral(L, "GraphicsParticleSystem"); lua_pushinteger(L, ComponentType::GraphicsParticleSystem); lua_settable(L, -3);
	lua_pushliteral(L, "Collision"); lua_pushinteger(L, ComponentType::Collision); lua_settable(L, -3);
	lua_pushliteral(L, "Wizard"); lua_pushinteger(L, ComponentType::Wizard); lua_settable(L, -3);
	lua_pushliteral(L, "AttributeStore"); lua_pushinteger(L, ComponentType::AttributeStore); lua_settable(L, -3);
	lua_setglobal(L, "ComponentType");

	auto callGetEntityAttributeValue = [](lua_State* s)->int {
		int argc = lua_gettop(s);
		if(argc != 2)
		{
			std::cerr << "callGetEntityAttributeValue: wrong number of arguments\n";
			return 0;		
		}
		ID entityID = lua_tonumber(s, 1);
		std::string key = lua_tostring(s, 2);
		Game* g = (Game*)lua_touserdata(s, lua_upvalueindex(1));
		auto e = g->_gameWorld.getEntity(entityID);
		AttributeStoreComponent* as = nullptr;
		if(e != nullptr && (as = e->getComponent<AttributeStoreComponent>()) != nullptr && as->hasAttribute(key)) {
			lua_pushnumber(s, as->getAttribute(key));
			lua_pushnumber(s, as->getAttributeAffected(key));
			return 2;
		}
		else
			return 0;
	};
	lua_pushlightuserdata(L, this);
	lua_pushcclosure(L, callGetEntityAttributeValue, 1);
	lua_setglobal(L, "getEntityAttributeValue");

	auto callSetEntityAttributeValue = [](lua_State* s)->int {
		int argc = lua_gettop(s);
		if(argc != 3)
		{
			std::cerr << "callSetEntityAttributeValue: wrong number of arguments\n";
			return 0;		
		}
		ID entityID = lua_tonumber(s, 1);
		std::string key = lua_tostring(s, 2);
		float val = lua_tonumber(s, 3);
		Game* g = (Game*)lua_touserdata(s, lua_upvalueindex(1));
		auto e = g->_gameWorld.getEntity(entityID);
		AttributeStoreComponent* as = nullptr;
		if(e != nullptr && (as = e->getComponent<AttributeStoreComponent>()) != nullptr)
			as->setOrAddAttribute(key, val);
		return 0;
	};
	lua_pushlightuserdata(L, this);
	lua_pushcclosure(L, callSetEntityAttributeValue, 1);
	lua_setglobal(L, "setEntityAttributeValue");

	auto callGetEntitiesByAttributeValue = [](lua_State* s)->int {
		int argc = lua_gettop(s);
		if(argc != 2 && argc != 1)
		{
			std::cerr << "callGetEntitiesByAttributeValue: wrong number of arguments\n";
			return 0;		
		}
		std::string attributeName = lua_tostring(s, 1);
		std::string attributeValue;
		if(argc == 2)
			attributeValue = lua_tostring(s, 2);
		Game* g = (Game*)lua_touserdata(s, lua_upvalueindex(1));
		std::vector<ID> entities;
		for(Entity& e : g->_gameWorld.getEntities())
		{
			AttributeStoreComponent* asc = e.getComponent<AttributeStoreComponent>();
			if(asc != nullptr)
			{
				if(asc->hasAttribute(attributeName))
					try {
						if(attributeValue == "" || asc->getAttribute(attributeName) == std::stof(attributeValue))
							entities.push_back(e.getID());
					}
				catch(std::invalid_argument& e) {
					std::cerr << "getEntitiesByAttributeValue: AttributeValue must be float or empty string.";
				}
			}
		}
		lua_createtable(s, entities.size(), 0);
		int newTable = lua_gettop(s);
		int index = 1;
		auto iter = entities.begin();
		while(iter != entities.end()) {
			lua_pushinteger(s, (*iter));
			lua_rawseti(s, newTable, index);
			++iter;
			++index;
		}
		return 1;
	};
	lua_pushlightuserdata(L, this);
	lua_pushcclosure(L, callGetEntitiesByAttributeValue, 1);
	lua_setglobal(L, "getEntitiesByAttributeValue");

	auto callGetEntityPosition = [](lua_State* s)->int {
		int argc = lua_gettop(s);
		if(argc != 1)
		{
			std::cerr << "callGetEntityPosition: wrong number of arguments\n";
			return 0;		
		}
		ID entityID = lua_tonumber(s, 1);
		Game* g = (Game*)lua_touserdata(s, lua_upvalueindex(1));
		auto e = g->_gameWorld.getEntity(entityID);
		BodyComponent* bc = nullptr;
		if(e != nullptr && (bc = e->getComponent<BodyComponent>()) != nullptr) {
			vec3f p = bc->getPosition();
			lua_pushnumber(s, p.X);
			lua_pushnumber(s, p.Y);
			lua_pushnumber(s, p.Z);
			return 3;
		}
		else
			return 0;
	};
	lua_pushlightuserdata(L, this);
	lua_pushcclosure(L, callGetEntityPosition, 1);
	lua_setglobal(L, "getEntityPosition");

	auto callGetAttributeModifierHistory = [](lua_State* s)->int {
		int argc = lua_gettop(s);
		if(argc != 1)
		{
			std::cerr << "callGetAttributeModifierHistory: wrong number of arguments\n";
			return 0;		
		}
		ID eID  = lua_tointeger(s, 1);
		Game* g = (Game*)lua_touserdata(s, lua_upvalueindex(1));
		Entity* e = g->_gameWorld.getEntity(eID);
		if(!e)
			return 0;
		AttributeStoreComponent* asc = e->getComponent<AttributeStoreComponent>();
		if(!asc)
			return 0;
		auto aaHistory = asc->getAttributeAffectorHistory();
		lua_newtable(s);
		int t = lua_gettop(s);
		int i = 1;
		for(AttributeAffector& aa : aaHistory)
		{
			lua_newtable(s);

			lua_pushstring(s, "author");
			lua_pushinteger(s, aa.getAuthor());
			lua_settable(s, -3);

			lua_pushstring(s, "attributeName");
			lua_pushstring(s, aa.getAffectedAttribute().c_str());
			lua_settable(s, -3);
			
			lua_pushstring(s, "modifierType");
			lua_pushinteger(s, aa.getModifierType());
			lua_settable(s, -3);

			lua_pushstring(s, "modifierValue");
			lua_pushnumber(s, aa.getModifierValue());
			lua_settable(s, -3);

			lua_pushstring(s, "permanent");
			lua_pushboolean(s, aa.isPermanent());
			lua_settable(s, -3);
			
			lua_rawseti(s, t, i);
			i++;
		}
		return 1;
	};
	lua_pushlightuserdata(L, this);
	lua_pushcclosure(L, callGetAttributeModifierHistory, 1);
	lua_setglobal(L, "getAttributeModifierHistory");

	auto callRemoveBodyComponent = [](lua_State* s)->int {
		int argc = lua_gettop(s);
		if(argc != 1)
		{
			std::cerr << "callRemoveBodyComponent: wrong number of arguments\n";
			return 0;		
		}
		ID entityID = lua_tonumber(s, 1);
		Game* g = (Game*)lua_touserdata(s, lua_upvalueindex(1));
		auto e = g->_gameWorld.getEntity(entityID);
		if(e)
			e->removeComponent<BodyComponent>();
		return 0;
	};
	lua_pushlightuserdata(L, this);
	lua_pushcclosure(L, callRemoveBodyComponent, 1);
	lua_setglobal(L, "removeBodyComponent");

	auto callAddBodyComponent = [](lua_State* s)->int {
		int argc = lua_gettop(s);
		if(argc != 4)
		{
			std::cerr << "callAddBodyComponent: wrong number of arguments\n";
			return 0;		
		}
		ID entityID = lua_tonumber(s, 1);
		float posX = lua_tonumber(s, 2);
		float posY = lua_tonumber(s, 3);
		float posZ = lua_tonumber(s, 4);
		std::cout << "adding bodyComponent: " << vec3f(posX, posY, posZ) << std::endl;
		Game* g = (Game*)lua_touserdata(s, lua_upvalueindex(1));
		auto e = g->_gameWorld.getEntity(entityID);
		if(e)
			e->addComponent<BodyComponent>(vec3f(posX,posY,posZ));
		return 0;
	};
	lua_pushlightuserdata(L, this);
	lua_pushcclosure(L, callAddBodyComponent, 1);
	lua_setglobal(L, "addBodyComponent");

	auto callSetGameRegValue = [](lua_State* s)->int {
		int argc = lua_gettop(s);
		if(argc != 2)
		{
			std::cerr << "callSetGameRegValue: wrong number of arguments\n";
			return 0;		
		}
		Game* g = (Game*)lua_touserdata(s, lua_upvalueindex(1));
		std::string key = lua_tostring(s, 1);
		if(lua_isstring(s, 2)) {
			std::string value = lua_tostring(s, 2);
			g->_registry.setValue(key, value);
		}
		else if(lua_isnumber(s, 2) || lua_isinteger(s, 2)) {
			float value;
			if(lua_isnumber(s, 2))
				value = lua_tonumber(s, 2);
			else
				value = lua_tointeger(s, 2);
			g->_registry.setValue(key, value);
		}
		else
			assert(false);
		return 0;
	};
	lua_pushlightuserdata(L, this);
	lua_pushcclosure(L, callSetGameRegValue, 1);
	lua_setglobal(L, "setGameRegValue");

	auto callEndRound = [](lua_State* s)->int {
		int argc = lua_gettop(s);
		if(argc != 0)
		{
			std::cerr << "callEndRound: wrong number of arguments\n";
			return 0;		
		}
		Game* g = (Game*)lua_touserdata(s, lua_upvalueindex(1));
		g->_ended = true;
		return 0;
	};
	lua_pushlightuserdata(L, this);
	lua_pushcclosure(L, callEndRound, 1);
	lua_setglobal(L, "endRound");
}

void Game::gameModeOnPlayerJoined(ID character)
{
	lua_State* L = _LuaStateGameMode;
	lua_getglobal(L, "onPlayerJoined");
	lua_pushinteger(L, character);
	if(lua_pcall(L, 1, 0, 0) != 0) {
		cerr << "something went wrong with onPlayerJoined: " << lua_tostring(L, -1) << endl;
		lua_pop(L, 1);
	}
}

void Game::gameModeOnPlayerLeft(ID character)
{
	lua_State* L = _LuaStateGameMode;
	lua_getglobal(L, "onPlayerLeft");
	lua_pushinteger(L, character);
	if(lua_pcall(L, 1, 0, 0) != 0) {
		cerr << "something went wrong with onPlayerLeft: " << lua_tostring(L, -1) << endl;
		lua_pop(L, 1);
	}
}

void Game::gameModeOnGameStart()
{
	lua_State* L = _LuaStateGameMode;
	lua_getglobal(L, "onGameStart");
	if(lua_pcall(L, 0, 0, 0) != 0) {
		cerr << "something went wrong with onGameStart: " << lua_tostring(L, -1) << endl;
	}
}

ID Game::addCharacter()
{
	ID character = _gameWorld.createCharacter(vec3f(0));
	gameModeOnPlayerJoined(character);
	return character;
}

void Game::removeCharacter(ID entityID)
{
	gameModeOnPlayerLeft(entityID);
	_gameWorld.removeEntity(entityID);
}

Entity* Game::getWorldEntity(ID eID)
{
	return _gameWorld.getEntity(eID);
}

void Game::handlePlayerCommand(Command& c, ID entity)
{
	_input.handleCommand(c, entity);
}

Game::Store& Game::getRegistry()
{
	return _registry;
}

void Game::gameModeOnEntityEvent(const EntityEvent& e)
{
	lua_State* L = _LuaStateGameMode;
	lua_getglobal(L, "onEntityEvent");
	lua_pushinteger(L, e.entityID);
	lua_pushinteger(L, e.componentT);
	lua_pushboolean(L, e.created);
	lua_pushboolean(L, e.destroyed);
	if(lua_pcall(L, 4, 0, 0) != 0) {
		cerr << "something went wrong with onEntityEvent: " << lua_tostring(L, -1) << endl;
		lua_pop(L, 1);
	}
}

void Game::GameModeEntityEventObserver::onMsg(const EntityEvent& e)
{
	_entityEventCallback(e);
}

Game::GameModeEntityEventObserver::GameModeEntityEventObserver(EntityEventCallback gameModeEntityEventCallback):
 	_entityEventCallback{gameModeEntityEventCallback}
{}

////////////////////////////////////////////////////////////

ServerApplication::ServerApplication(IrrlichtDevice* irrDev)
	: _irrDevice{irrDev}, _map{vec2u(64),
#ifdef DEBUG_BUILD
	1
#else
	std::random_device()()
#endif
	}
	,
	_updater(std::bind(&ServerApplication::send, ref(*this), placeholders::_1, placeholders::_2),
			std::bind(&Game::getWorldEntity, ref(_game), placeholders::_1))
{
	_listener.setBlocking(false);
	newGame();
}

bool ServerApplication::listen(short port)
{
	return _listener.listen(port) == sf::Socket::Done;
}

void ServerApplication::run()
{
	sf::Clock c;
	while(true)
	{
		acceptClient();
		for(auto s = _sessions.begin(); s != _sessions.end(); s++)
		{
			while(s->receive());
			if(s->isClosed())
			{
				onClientDisconnect(_sessions.iteratorToIndex(s));
				break;
			}
		}

		float timeDelta = c.restart().asSeconds();

		if(_game)
			if(!_game->run(timeDelta))
				newGame();
		_updater.tick(timeDelta);

		sf::sleep(sf::milliseconds(50));
		_irrDevice->getVideoDriver()->endScene();
	}
}

void ServerApplication::acceptClient()
{
	unique_ptr<sf::TcpSocket> sock(new sf::TcpSocket);
	sock->setBlocking(false);
	
	if(_listener.accept(*sock) == sf::Socket::Done)
	{
		onClientConnect(std::move(sock));
	}
}

void ServerApplication::send(sf::Packet& p, Updater::ClientFilterPredicate fp)
{
	for(auto& s : _sessions)
		if(fp(s.getControlledObjID()))
			s.send(p);
}

void ServerApplication::onClientConnect(std::unique_ptr<sf::TcpSocket>&& sock)
{
	cout << "Client connected from " << sock->getRemoteAddress() << endl;
	auto sID = _sessions.emplace(std::move(sock));
	_sessions[sID].setOnAuthorized([this, sID](){ onClientAuthorized(sID); });
}

void ServerApplication::onClientAuthorized(ID sessionID)
{
	Session& s = _sessions[sessionID];
	joinGame(s);
}

void ServerApplication::joinGame(Session& s)
{
	if(_game)
	{
		s.setCommandHandler([this](Command& c, ID objID){
				_game->handlePlayerCommand(c, objID);
				});
		_game->getRegistry().addObserver(s);
		sendMapTo(s);
		_game->sendHelloMsgTo(_updater); // TODO send updates only to newly connected client
		s.setControlledObjID(_game->addCharacter());
	}
}

void ServerApplication::onClientDisconnect(ID sessionID)
{
	Session& s = _sessions[sessionID];
	cout << "Client disconnected: " << s.getSocket().getRemoteAddress() << endl;
	if(_game)
		_game->removeCharacter(s.getControlledObjID());
	_sessions.remove(sessionID);
}

ServerApplication::~ServerApplication()
{
	_listener.close();
}

void ServerApplication::sendMapTo(Session& client)
{
	sf::Packet p;
	p << PacketType::GameInit << Serializer<sf::Packet>(_map);
	client.send(p);
}

void ServerApplication::newGame()
{
	_game.reset(new Game(_map));
	_game->addObserver(_updater);
	for(Session& s : _sessions)
		joinGame(s);
}
