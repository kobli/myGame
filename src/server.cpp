#include <server.hpp>
#include <network.hpp>
#include <cassert>
#include <serdes.hpp>

Session::Session(unique_ptr<sf::TcpSocket>&& socket, CommandHandler h)
	: _socket{std::move(socket)}, _commandHandler{h}, _closed{false}
{
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
	return static_cast<ID>(getValue("controlled_object_id"));
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
	{
		cerr << "Attept to send data through a socket that was not ready.\n";
		_closed = true;
	}
}

void Session::handlePacket(sf::Packet& p)
{
	PacketType pt;
	p >> pt;
	switch(pt)
	{
		case PacketType::PlayerCommand:
		{
			Command c;
			p >> c;
			//cout << "command type: " << unsigned(c._type) << endl;
			_commandHandler(c, getControlledObjID());
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

void Session::updateClientSharedRegistry()
{
	sf::Packet p;
	p << PacketType::RegistryUpdate << Serializer<sf::Packet>(*static_cast<KeyValueStore*>(this));
	send(p);
}

void Session::addPair(std::string key, float value)
{
	KeyValueStore::addPair(key, value);
	updateClientSharedRegistry();
}

void Session::setValue(std::string key, float value)
{
	KeyValueStore::setValue(key, value);
	updateClientSharedRegistry();
}

////////////////////////////////////////////////////////////

Updater::Updater(Sender s, EntityResolver getEntity): _send{s}, _getEntity{getEntity}
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
	_send(p, [](Entity*){ return true; });

	/*
	cout << "sent an update:\n\tentityID: " << e.entityID 
		<< "\n\tcomponent modified type: " << e.componentT << endl;
	if(modifiedComponent != nullptr && !e.destroyed)
		cout << "\tcomponent: " << Serializer<ostream>(*modifiedComponent) << endl;
		*/
}

////////////////////////////////////////////////////////////


ServerApplication::ServerApplication(IrrlichtDevice* irrDev)
	: _irrDevice{irrDev}, _map{vec2u(64),9}, _gameWorld{_map}
	, _physics{_gameWorld}, _spells{_gameWorld}, _input{_gameWorld, _spells},
	_updater(std::bind(&ServerApplication::send, ref(*this), placeholders::_1, placeholders::_2),
			std::bind(&World::getEntity, ref(_gameWorld), placeholders::_1)), _LuaStateGameMode{nullptr},
	_gameModeEntityEventObserver{[this](const EntityEvent& e){ this->gameModeOnEntityEvent(e); }}
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

	_listener.setBlocking(false);
	_gameWorld.addObserver(*this);
	_physics.registerCollisionCallback(std::bind(&SpellSystem::collisionCallback, std::ref(_spells), placeholders::_1, placeholders::_2));

	_LuaStateGameMode = luaL_newstate();
	luaL_openlibs(_LuaStateGameMode);
	gameModeRegisterAPIMethods();
	if(luaL_dofile(_LuaStateGameMode, "lua/gamemode_dm.lua"))
		printf("%s\n", lua_tostring(_LuaStateGameMode, -1));
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
		while(!_eventQueue.empty()) {
			EntityEvent e = _eventQueue.front();
			_eventQueue.pop();
			_updater.onMsg(e);
			_spells.onMsg(e);
			_physics.onMsg(e);
			_gameModeEntityEventObserver.onMsg(e);
		}

		float timeDelta = c.restart().asSeconds();
		_physics.update(timeDelta);
		_spells.update(timeDelta);
		_updater.tick(timeDelta);

		sf::sleep(sf::milliseconds(50));
		_irrDevice->getVideoDriver()->endScene();
	}
}

void ServerApplication::onMsg(const EntityEvent& m)
{
	_eventQueue.push(m);
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
		if(fp(_gameWorld.getEntity(s.getControlledObjID())))
			s.send(p);
}

void ServerApplication::onClientConnect(std::unique_ptr<sf::TcpSocket>&& sock)
{
	cout << "Client connected from " << sock->getRemoteAddress() << endl;
	auto sID = _sessions.emplace(std::move(sock));
	_sessions[sID].setCommandHandler([this](Command& c, ID objID){
			if(objID != NULLID)
				_input.handleCommand(c, objID);
			});
	sendMapTo(_sessions[sID]);
	_gameWorld.sendHelloMsgTo(_updater); // TODO send updates only to newly connected client
	gameModeOnClientConnect(sID);
}

void ServerApplication::onClientDisconnect(ID sessionID)
{
	Session& s = _sessions[sessionID];
	cout << "Client disconnected: " << s.getSocket().getRemoteAddress() << endl;
	gameModeOnClientDisconnect(sessionID);
	_sessions.remove(sessionID);
}

ServerApplication::~ServerApplication()
{
	_listener.close();
	lua_close(_LuaStateGameMode);
}

void ServerApplication::gameModeRegisterAPIMethods()
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

	auto callCreateCharacter = [](lua_State* s)->int {
		int argc = lua_gettop(s);
		if(argc != 3)
		{
			std::cerr << "callCreateCharacter: wrong number of arguments\n";
			return 0;		
		}
		vec3f pos;
		pos.X = lua_tonumber(s, 1);
		pos.Y = lua_tonumber(s, 2);
		pos.Z = lua_tonumber(s, 3);
		ServerApplication* sApp = (ServerApplication*)lua_touserdata(s, lua_upvalueindex(1));
		ID characterID = sApp->_gameWorld.createCharacter(pos);
		lua_pushinteger(s, characterID);
		return 1;
	};
	lua_pushlightuserdata(L, this);
	lua_pushcclosure(L, callCreateCharacter, 1);
	lua_setglobal(L, "createCharacter");

	auto callRemoveWorldEntity = [](lua_State* s)->int {
		int argc = lua_gettop(s);
		if(argc != 1)
		{
			std::cerr << "callRemoveWorldEntity: wrong number of arguments\n";
			return 0;		
		}
		ID entID = lua_tonumber(s, 1);
		ServerApplication* sApp = (ServerApplication*)lua_touserdata(s, lua_upvalueindex(1));
		sApp->_gameWorld.removeEntity(entID);
		return 0;
	};
	lua_pushlightuserdata(L, this);
	lua_pushcclosure(L, callRemoveWorldEntity, 1);
	lua_setglobal(L, "removeWorldEntity");

	auto callSetClientControlledObjectID= [](lua_State* s)->int {
		int argc = lua_gettop(s);
		if(argc != 2)
		{
			std::cerr << "callSetClientControlledObjectID: wrong number of arguments\n";
			return 0;		
		}
		ID sessionID = lua_tonumber(s, 1);
		ID objID = lua_tonumber(s, 2);
		ServerApplication* sApp = (ServerApplication*)lua_touserdata(s, lua_upvalueindex(1));
		if(sApp->_sessions.indexValid(sessionID))
			sApp->_sessions[sessionID].setControlledObjID(objID);
		return 0;
	};
	lua_pushlightuserdata(L, this);
	lua_pushcclosure(L, callSetClientControlledObjectID, 1);
	lua_setglobal(L, "setClientControlledObjectID");

	auto callGetClientControlledObjectID= [](lua_State* s)->int {
		int argc = lua_gettop(s);
		if(argc != 1)
		{
			std::cerr << "callGetClientControlledObjectID: wrong number of arguments\n";
			return 0;		
		}
		ID sessionID = lua_tonumber(s, 1);
		ServerApplication* sApp = (ServerApplication*)lua_touserdata(s, lua_upvalueindex(1));
		if(sApp->_sessions.indexValid(sessionID)) {
			lua_pushinteger(s, sApp->_sessions[sessionID].getControlledObjID());
			return 1;
		}
		else
			return 0;
	};
	lua_pushlightuserdata(L, this);
	lua_pushcclosure(L, callGetClientControlledObjectID, 1);
	lua_setglobal(L, "getClientControlledObjectID");

	auto callGetEntityAttributeValue = [](lua_State* s)->int {
		int argc = lua_gettop(s);
		if(argc != 2)
		{
			std::cerr << "callGetEntityAttributeValue: wrong number of arguments\n";
			return 0;		
		}
		ID entityID = lua_tonumber(s, 1);
		std::string key = lua_tostring(s, 2);
		ServerApplication* sApp = (ServerApplication*)lua_touserdata(s, lua_upvalueindex(1));
		auto e = sApp->_gameWorld.getEntity(entityID);
		AttributeStoreComponent* as = nullptr;
		if(e != nullptr && (as = e->getComponent<AttributeStoreComponent>()) != nullptr) {
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
		ServerApplication* sApp = (ServerApplication*)lua_touserdata(s, lua_upvalueindex(1));
		auto e = sApp->_gameWorld.getEntity(entityID);
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
		ServerApplication* sApp = (ServerApplication*)lua_touserdata(s, lua_upvalueindex(1));
		std::vector<ID> entities;
		for(Entity& e : sApp->_gameWorld.getEntities())
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
		ServerApplication* sApp = (ServerApplication*)lua_touserdata(s, lua_upvalueindex(1));
		auto e = sApp->_gameWorld.getEntity(entityID);
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
}

void ServerApplication::gameModeOnClientConnect(ID sessionID)
{
	lua_State* L = _LuaStateGameMode;
	lua_getglobal(L, "onClientConnect");
	lua_pushinteger(L, sessionID);
	if(lua_pcall(L, 1, 0, 0) != 0) {
		cerr << "something went wrong with onClientConnect: " << lua_tostring(L, -1) << endl;
		lua_pop(L, 1);
	}
}

void ServerApplication::gameModeOnClientDisconnect(ID sessionID)
{
	lua_State* L = _LuaStateGameMode;
	lua_getglobal(L, "onClientDisconnect");
	lua_pushinteger(L, sessionID);
	if(lua_pcall(L, 1, 0, 0) != 0) {
		cerr << "something went wrong with onClientDisconnect: " << lua_tostring(L, -1) << endl;
		lua_pop(L, 1);
	}
}

void ServerApplication::gameModeOnEntityEvent(const EntityEvent& e)
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

void ServerApplication::GameModeEntityEventObserver::onMsg(const EntityEvent& e)
{
	_entityEventCallback(e);
}

ServerApplication::GameModeEntityEventObserver::GameModeEntityEventObserver(EntityEventCallback gameModeEntityEventCallback):
 	_entityEventCallback{gameModeEntityEventCallback}
{}

void ServerApplication::sendMapTo(Session& client)
{
	sf::Packet p;
	p << PacketType::GameInit << Serializer<sf::Packet>(_map);
	client.send(p);
}
