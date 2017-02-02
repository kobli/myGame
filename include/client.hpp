#include <main.hpp>
#include <SFML/Network.hpp>
#include <world.hpp>
#include <system.hpp>

#ifndef CLIENT_HPP_16_11_26_10_46_45

class Animator: public Observer<EntityEvent>
{
	public:
		Animator(std::function<WorldEntity*(u32 ID)> entityResolver = [](u32){ return nullptr; });
		void setEntityResolver(std::function<WorldEntity*(u32 ID)> entityResolver);

	private:
		void onObservableAdd(EntityEvent& m);
		void onObservableUpdate(EntityEvent& m);
		void onObservableRemove(EntityEvent& m);

		function<WorldEntity*(u32)> _entityResolver;
};

////////////////////////////////////////////////////////////

class ClientApplication
{
	public:
		ClientApplication();
		bool connect(string host, short port);
		void run();
		void createWorld();
		void createCamera();
		
	private:
		sf::TcpSocket _server;
		Controller _controller;
		unique_ptr<IrrlichtDevice, void(*)(IrrlichtDevice*)> _device;
		unique_ptr<WorldMap> _worldMap;
		unique_ptr<World> _gameWorld;
		unique_ptr<ViewSystem> _vs;
		unique_ptr<Physics> _physics;
		Animator _animator;
		scene::ICameraSceneNode* _camera;

		void sendCommand(Command& c);
		void sendPacket(sf::Packet& p);
		bool receive();
		void handlePacket(sf::Packet& p);
		void handleEntityEvent(EntityEvent& e);
};


#define CLIENT_HPP_16_11_26_10_46_45 
#endif /* CLIENT_HPP_16_11_26_10_46_45 */
