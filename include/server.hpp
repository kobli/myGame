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
		const WorldMap& getMap() const;

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

// receives and processes data
class Session: public Observer<KeyValueStoreChange<PacketType>>
{
		using Store = ObservableKeyValueStore<PacketType,PacketType::RegistryUpdate>;
		using MessageT = Store::MessageT;

	public:
		using GameJoinRequestHandler = std::function<bool(Session& s)>;
		Session(unique_ptr<sf::TcpSocket>&& socket, GameJoinRequestHandler h);
		Session(const Session&) = delete;
		Session& operator=(const Session&) = delete;
		Session(Session&&);
		Session& operator=(Session&&);
		virtual void swap(Session& other);
		~Session();
		sf::TcpSocket& getSocket();
		bool receive();
		void send(sf::Packet& p);
		void send(PacketType t);
		bool isClosed();
		template <typename T>
		void setValue(std::string key, T value);
		virtual void onMsg(const MessageT& m) final;
		void joinGame(Game& game);
		void leaveGame();
		ID getControlledObjID() const;
		std::string getRemoteAddress() const;

	private:
		Session();
		Game* _game;
		Store _sharedRegistry;
		GameJoinRequestHandler _requestGameJoin;
		unique_ptr<sf::TcpSocket> _socket;
		void handlePacket(sf::Packet& p);
		bool _closed;
		bool _authorized;

		void addPair(std::string key, float value);
		void disconnectUnauthorized(std::string reason = "Unauthorized.");
		void onAuthorized();
		void sendMap(const WorldMap& map);

		void setControlledObjID(ID id);
};

void swap(Session& lhs, Session& rhs);
std::ostream& operator<<(std::ostream& o, const Session& s);

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
		void reset();

	private:
		Sender _send;
		EntityResolver _getEntity;
		std::unordered_set<EntityEvent> _updateEventQueue;
		float _timeSinceLastUpdateSent;

		void sendEvent(const EntityEvent&);
};

////////////////////////////////////////////////////////////

class ServerApplication
{
	public:
		ServerApplication(IrrlichtDevice* irrDev);
		bool listen(short port);
		void run();
		~ServerApplication();
		bool requestGameJoin(Session& s);

	private:
		void acceptClient();
		void onClientConnect(unique_ptr<sf::TcpSocket>&& s);
		void onClientDisconnect(ID sessionID);
		void broadcast(sf::Packet& p, Updater::ClientFilterPredicate fp);
		void gameOver();
		void newGame();

		sf::TcpListener _listener;
		SolidVector<Session,ID,NULLID> _sessions;
		IrrlichtDevice* _irrDevice;

		WorldMap _map;
		std::unique_ptr<Game> _game;

		Updater _updater;
};

#endif /* SERVER_HPP_16_11_26_09_22_02 */
