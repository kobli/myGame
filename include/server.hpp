#include <memory>
#include <unordered_set>
#include <main.hpp>
#include <SFML/Network.hpp>
#include <controller.hpp>
#include <world.hpp>
#include <system.hpp>
#include "keyValueStore.hpp"
#include <queue>

#ifndef SERVER_HPP_16_11_26_09_22_02
#define SERVER_HPP_16_11_26_09_22_02 

// receives and processes data
class Session: KeyValueStore
{
	public:
		using CommandHandler = std::function<void(Command& c, ID charID)>;
		Session(unique_ptr<sf::TcpSocket>&& socket, CommandHandler commandHandler = [](Command&, ID){});
		sf::TcpSocket& getSocket();
		void setCommandHandler(CommandHandler h);
		ID getControlledObjID();
		void setControlledObjID(ID objID);
		bool receive();
		void send(sf::Packet& p);
		bool isClosed();

	private:
		unique_ptr<sf::TcpSocket> _socket;
		CommandHandler _commandHandler;
		void handlePacket(sf::Packet& p);
		bool _closed;

		void updateClientSharedRegistry();
		void addPair(std::string key, float value);
		void setValue(std::string key, float value);
};

////////////////////////////////////////////////////////////

// decides, which clients should be informed about a change in the world
// -> builds a packet 
// for example by visibility check, areas, or simply send to all
//TODO rename
class Updater: public Observer<EntityEvent>
{
	public:
		using ClientFilterPredicate = function<bool(Entity* e)>; 
			// decides, if the client controlling entity "e" should be informed about these WorldChanges
		using Sender = function<void(sf::Packet& p, ClientFilterPredicate fp)>;
		using EntityResolver = function<Entity*(ID entID)>;
		Updater(Sender s, EntityResolver getEntity);
		void tick(float delta);
		void onMsg(const EntityEvent& m) final;

	private:
		Sender _send;
		EntityResolver _getEntity;
		std::unordered_set<EntityEvent> _updateEventQueue;
		float _timeSinceLastUpdateSent;

		void sendEvent(const EntityEvent&);
};

////////////////////////////////////////////////////////////

class ServerApplication: private Observer<EntityEvent>
{
	public:
		ServerApplication(IrrlichtDevice* irrDev);
		bool listen(short port);
		void run();
		~ServerApplication();

	private:
		virtual void onMsg(const EntityEvent& m) override;
		void acceptClient();
		void onClientConnect(unique_ptr<sf::TcpSocket>&& s);
		void onClientDisconnect(ID sessionID);
		void send(sf::Packet& p, Updater::ClientFilterPredicate fp);
		void gameModeRegisterAPIMethods();
		void gameModeOnClientConnect(ID sessionID);
		void gameModeOnClientDisconnect(ID sessionID);
		void gameModeOnEntityEvent(const EntityEvent& e);
		void sendMapTo(Session& client);

		sf::TcpListener _listener;
		SolidVector<Session,ID,NULLID> _sessions;
		IrrlichtDevice* _irrDevice;
		WorldMap _map;
		World _gameWorld;
		Physics _physics;
		SpellSystem _spells;
		InputSystem _input;
		Updater _updater;
		lua_State* _LuaStateGameMode;
		std::queue<EntityEvent> _eventQueue;

		class GameModeEntityEventObserver: public Observer<EntityEvent> {
			typedef std::function<void(const EntityEvent& e)> EntityEventCallback;
			EntityEventCallback _entityEventCallback;
			public:
				GameModeEntityEventObserver(EntityEventCallback gameModeEntityEventCallback);
				void onMsg(const EntityEvent& e) final;
		};
		GameModeEntityEventObserver _gameModeEntityEventObserver;
};

#endif /* SERVER_HPP_16_11_26_09_22_02 */
