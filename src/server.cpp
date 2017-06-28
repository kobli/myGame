#include <server.hpp>
#include <network.hpp>
#include <cassert>
#include <serdes.hpp>

Session::Session(unique_ptr<sf::TcpSocket>&& socket, ID controlledObjID, CommandHandler h)
	: _socket{std::move(socket)}, _commandHandler{h}, _closed{false}
{
	addPair("controlled_object_id", static_cast<float>(controlledObjID));
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
	if(_timeSinceLastUpdateSent >= 0.1) {
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

	cout << "sent an update:\n\tentityID: " << e.entityID 
		<< "\n\tcomponent modified type: " << e.componentT << endl;
	if(modifiedComponent != nullptr && !e.destroyed)
		cout << "\tcomponent: " << Serializer<ostream>(*modifiedComponent) << endl;
}

////////////////////////////////////////////////////////////


ServerApplication::ServerApplication(IrrlichtDevice* irrDev)
	: _irrDevice{irrDev}, _map{70, irrDev->getSceneManager()->createNewSceneManager()}, _gameWorld{_map}
	, _physics{_gameWorld}, _spells{_gameWorld}, _input{_gameWorld, _spells},
	_updater(std::bind(&ServerApplication::send, ref(*this), placeholders::_1, placeholders::_2),
			std::bind(&World::getEntity, ref(_gameWorld), placeholders::_1))
{
	_listener.setBlocking(false);
	_gameWorld.addObserver(_updater);
	_gameWorld.addObserver(_spells);
	_gameWorld.addObserver(_physics);
	_physics.registerCollisionCallback(std::bind(&SpellSystem::collisionCallback, std::ref(_spells), placeholders::_1, placeholders::_2));
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
		for(auto& s : _sessions)
		{
			while(s.receive());
			if(s.isClosed())
			{
				onClientDisconnect(s);
				break;
			}
		}
		float timeDelta = c.restart().asSeconds();
		_physics.update(timeDelta);
		_spells.update(timeDelta);
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
		if(fp(_gameWorld.getEntity(s.getControlledObjID())))
			s.send(p);
}

void ServerApplication::onClientConnect(std::unique_ptr<sf::TcpSocket>&& sock)
{
	cout << "Client connected from " << sock->getRemoteAddress() << endl;
	ID cID = _gameWorld.createCharacter(vec3f(0,50,0));
	auto& e = *_gameWorld.getEntity(cID);
	_sessions.emplace_back(std::move(sock), e.getID());
	_sessions.back().setCommandHandler([this](Command& c, u32 objID){
			_input.handleCommand(c, objID);
			});
	_gameWorld.sendHelloMsgTo(_updater); // TODO send updates only to newly connected client
}

void ServerApplication::onClientDisconnect(Session& s)
{
	cout << "Client disconnected: " << s.getSocket().getRemoteAddress() << endl;
	auto* cc = _gameWorld.getEntity(s.getControlledObjID());
	auto it = _sessions.begin();
	while(&*it != &s && it != _sessions.end())
		it++;
	_sessions.erase(it);
	if(cc)
		_gameWorld.removeEntity(cc->getID());
}

ServerApplication::~ServerApplication()
{
	_listener.close();
}
