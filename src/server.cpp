#include <server.hpp>
#include <network.hpp>
#include <cassert>
#include <serdes.hpp>

Session::Session(unique_ptr<sf::TcpSocket>&& socket, WorldEntity* controlledCharacter)
	: _socket{std::move(socket)}, _controlledCharacter{controlledCharacter}, _closed{false}
{}

sf::TcpSocket& Session::getSocket()
{
	return *_socket;
}

WorldEntity* Session::getControlledCharacter()
{
	return _controlledCharacter;
}

void Session::setControlledCharacter(WorldEntity* character)
{
	_controlledCharacter = character;
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
		//cerr << "Attept to receive data from a socket that was not ready.\n";
		//_closed = true;
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
			cout << "command type: " << unsigned(c._type) << endl;
			if(_controlledCharacter && _controlledCharacter->getInputComponent())
				_controlledCharacter->getInputComponent()->handleCommand(c);
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


////////////////////////////////////////////////////////////

Updater::Updater(Sender s): _send{s}
{}

void Updater::onObservableAdd(EntityEvent& m)
{
	onObservableUpdate(m);
}

void Updater::onObservableUpdate(EntityEvent& m)
{
	// do not send position and rotation updates
	if(m._componentModifiedType == ComponentType::Body && m._componentModified && !m._destroyed
			&& static_cast<BodyComponent*>(m._componentModified)->posOrRotChanged())
		return;
	//TODO send destroy updates
	sf::Packet p;
	p << PacketType::WorldUpdate << m;
	if(m._componentModified && !m._destroyed)
		p << Serializer<sf::Packet>(*m._componentModified);
	_send(p, [](WorldEntity*){ return true; });
	cout << "sent an update:\n\tentityID: " << m._entityID 
		<< "\n\tcomponent modified type: " << m._componentModifiedType << endl;
	if(m._componentModified != nullptr && !m._destroyed)
		cout << "\tcomponent: " << Serializer<ostream>(*m._componentModified) << endl;
}

void Updater::onObservableRemove(EntityEvent&)
{}

////////////////////////////////////////////////////////////


ServerApplication::ServerApplication(IrrlichtDevice* irrDev)
	: _irrDevice{irrDev}, _map{irrDev->getSceneManager()->createNewSceneManager()}, _gameWorld{_map}
	, _updater(std::bind(&ServerApplication::send, ref(*this), placeholders::_1, placeholders::_2))
{
	_listener.setBlocking(false);
	_updater.observe(_gameWorld);
}

bool ServerApplication::listen(short port)
{
	return _listener.listen(port) == sf::Socket::Done;
}

void ServerApplication::run()
{
	//auto driver = _irrDevice->getVideoDriver();
	sf::Clock c;
	while(true)
	{
		//driver->beginScene();
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
		//_gameWorld.update(1./driver->getFPS());
		_gameWorld.update(c.restart().asSeconds());
		sf::sleep(sf::milliseconds(40));
		//driver->endScene();
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
		if(fp(s.getControlledCharacter()))
			s.send(p);
}

void ServerApplication::onClientConnect(std::unique_ptr<sf::TcpSocket>&& sock)
{
	cout << "Client connected from " << sock->getRemoteAddress() << endl;
	//TODO flip following lines!!
	_sessions.emplace_back(std::move(sock));
	auto& e = _gameWorld.createCharacter(vec3f(2700*2,255*2,2600*2)+vec3f(50,200,0));
	_sessions.back().setControlledCharacter(&e);
	_gameWorld.sendAddMsg(_updater); // TODO send updates only to newly connected client
}

void ServerApplication::onClientDisconnect(Session& s)
{
	cout << "Client disconnected: " << s.getSocket().getRemoteAddress() << endl;
	auto* cc = s.getControlledCharacter();
	auto it = _sessions.begin();
	while(&*it != &s && it != _sessions.end())
		it++;
	_sessions.erase(it);
	if(cc)
		_gameWorld.removeEntity(*cc);
}

ServerApplication::~ServerApplication()
{
	_listener.close();
}
