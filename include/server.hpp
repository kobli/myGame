#include <memory>
#include <unordered_set>
#include <main.hpp>
#include <SFML/Network.hpp>
#include <controller.hpp>
#include <world.hpp>
#include <system.hpp>
#include "keyValueStore.hpp"
#include "observableKeyValueStore.hpp"
#include "network.hpp"
#include <queue>

#ifndef SERVER_HPP_16_11_26_09_22_02
#define SERVER_HPP_16_11_26_09_22_02 

// receives and processes data
class Session: ObservableKeyValueStore<PacketType,PacketType::RegistryUpdate>, public Observer<KeyValueStoreChange<PacketType>>
{
	public:
		using CommandHandler = std::function<void(Command& c, ID charID)>;
		using OnAuthorized = std::function<void(void)>;
		Session(unique_ptr<sf::TcpSocket>&& socket, CommandHandler commandHandler = [](Command&, ID){});
		sf::TcpSocket& getSocket();
		void setCommandHandler(CommandHandler h);
		ID getControlledObjID();
		void setControlledObjID(ID objID);
		bool receive();
		void send(sf::Packet& p);
		bool isClosed();
		void setOnAuthorized(OnAuthorized cb);
		template <typename T>
		void setValue(std::string key, T value);
		virtual void onMsg(const MessageT& m) final;

	private:
		using Store = ObservableKeyValueStore<PacketType,PacketType::RegistryUpdate>;
		unique_ptr<sf::TcpSocket> _socket;
		CommandHandler _commandHandler;
		void handlePacket(sf::Packet& p);
		bool _closed;
		bool _authorized;
		OnAuthorized _onAuthorized;

		void addPair(std::string key, float value);
		void disconnectUnauthorized(std::string reason = "Unauthorized.");
};

////////////////////////////////////////////////////////////

// decides, which clients should be informed about a change in the world
// -> builds a packet 
// for example by visibility check, areas, or simply send to all
//TODO rename
class Updater: public Observer<EntityEvent>
{
	public:
		using ClientFilterPredicate = function<bool(ID e)>; 
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

class Game: public Observabler<EntityEvent>
{
	public:
		Game(const WorldMap& map);
		~Game();

		bool run(float timeDelta);
		void onMessage(const EntityEvent& m);
		ID addCharacter();
		void removeCharacter(ID entityID);
		Entity* getWorldEntity(ID eID);
		void handlePlayerCommand(Command& c, ID entity);
		using Store = ObservableKeyValueStore<PacketType,PacketType::GameRegistryUpdate>;
		Store& getRegistry();

	private:
		void loadMap();

		void gameModeRegisterAPIMethods();
		void gameModeOnEntityEvent(const EntityEvent& e);
		void gameModeOnPlayerJoined(ID character);
		void gameModeOnPlayerLeft(ID character);
		void gameModeOnGameStart();

		const WorldMap& _map;
		World _gameWorld;
		Physics _physics;
		SpellSystem _spells;
		InputSystem _input;
		lua_State* _LuaStateGameMode;
		std::queue<EntityEvent> _eventQueue;
		Store _registry;

		class GameModeEntityEventObserver: public Observer<EntityEvent> {
			typedef std::function<void(const EntityEvent& e)> EntityEventCallback;
			EntityEventCallback _entityEventCallback;
			public:
				GameModeEntityEventObserver(EntityEventCallback gameModeEntityEventCallback);
				void onMsg(const EntityEvent& e) final;
		};
		GameModeEntityEventObserver _gameModeEntityEventObserver;
		bool _ended;
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
		void onClientDisconnect(ID sessionID);
		void send(sf::Packet& p, Updater::ClientFilterPredicate fp);
		void onClientAuthorized(ID sessionID);
		void sendMapTo(Session& client);
		void joinGame(Session& s);
		void newGame();

		sf::TcpListener _listener;
		SolidVector<Session,ID,NULLID> _sessions;
		IrrlichtDevice* _irrDevice;

		WorldMap _map;
		std::unique_ptr<Game> _game;

		Updater _updater;
};

#endif /* SERVER_HPP_16_11_26_09_22_02 */
