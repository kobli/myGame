#include <memory>
#include <main.hpp>
#include <SFML/Network.hpp>
#include <controller.hpp>
#include <world.hpp>
#include <system.hpp>

#ifndef SERVER_HPP_16_11_26_09_22_02
#define SERVER_HPP_16_11_26_09_22_02 

// receives and processes data
class Session
{
	public:
		Session(unique_ptr<sf::TcpSocket>&& socket, WorldEntity* controlledCharacter = nullptr);
		sf::TcpSocket& getSocket();
		WorldEntity* getControlledCharacter();
		void setControlledCharacter(WorldEntity* character);
		bool receive();
		void send(sf::Packet& p);
		bool isClosed();

	private:
		unique_ptr<sf::TcpSocket> _socket;
		WorldEntity* _controlledCharacter;
		void handlePacket(sf::Packet& p);
		bool _closed;
};

////////////////////////////////////////////////////////////

// decides, which clients should be informed about a change in the world
// -> builds a packet 
// for example by visibility check, areas, or simply send to all
//TODO rename
class Updater: public Observer<EntityEvent>
{
	public:
		using ClientFilterPredicate = function<bool(WorldEntity* e)>; 
			// decides, if the client controlling entity "e" should be informed about these WorldChanges
		using Sender = function<void(sf::Packet& p, ClientFilterPredicate fp)>;
		Updater(Sender s);

	private:
		Sender _send;

		void onObservableAdd(EntityEvent& m);
		void onObservableUpdate(EntityEvent& m);
		void onObservableRemove(EntityEvent& m);
};

////////////////////////////////////////////////////////////

class ServerApplication
{
	public:
		ServerApplication(IrrlichtDevice* irrDev);
		bool listen(short port);
		void run();
		~ServerApplication();

	private:
		void acceptClient();
		void onClientConnect(unique_ptr<sf::TcpSocket>&& s);
		void onClientDisconnect(Session& s);
		void send(sf::Packet& p, Updater::ClientFilterPredicate fp);

		sf::TcpListener _listener;
		std::list<Session> _sessions;
		IrrlichtDevice* _irrDevice;
		WorldMap _map;
		World _gameWorld;
		Physics _physics;
		Updater _updater;
};

#endif /* SERVER_HPP_16_11_26_09_22_02 */
