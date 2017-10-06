#ifndef CLIENT_HPP_16_11_26_10_46_45
#define CLIENT_HPP_16_11_26_10_46_45 
#include <SFML/Network.hpp>
#include "main.hpp"
#include "world.hpp"
#include "system.hpp"
#include "keyValueStore.hpp"
#include "timedFilter.hpp"
#include "gui.hpp"

class Animator: public Observer<EntityEvent>
{
	public:
		Animator(scene::ISceneManager* smgr = nullptr, function<Entity*(ID)> entityResolver = [](ID){ return nullptr; }
				, function<vec3f(ID)> entityVelocityGetter = function<vec3f(ID)>());
		void setEntityResolver(std::function<Entity*(ID id)> entityResolver);
		void setEntityVelocityGetter(std::function<vec3f(ID id)> entityVelocityGetter);
		void setSceneManager(scene::ISceneManager* smgr);

	private:
		scene::ISceneManager* _smgr;
		function<Entity*(ID)> _entityResolver;
		std::function<vec3f(ID)> _velGetter;
		void onMsg(const EntityEvent& m);

};

////////////////////////////////////////////////////////////

class ClientApplication
{
	public:
		ClientApplication();
		bool connect(string host, unsigned short port);
		void run();
		void startGame();
		void createCamera();
		
	private:
		sf::TcpSocket _server;
		unique_ptr<IrrlichtDevice, void(*)(IrrlichtDevice*)> _device;
		Controller _controller;
		unique_ptr<WorldMap> _worldMap;
		unique_ptr<World> _gameWorld;
		unique_ptr<ViewSystem> _vs;
		unique_ptr<Physics> _physics;
		unique_ptr<GUI> _gui;
		Animator _animator;
		KeyValueStore _sharedRegistry;
		KeyValueStore _gameRegistry;
		float _cameraElevation;
		float _cameraYAngle;
		TimedFilter<float> _yAngleSetCommandFilter;

		void commandHandler(Command& c);
		void sendCommand(Command& c);
		void sendPacket(sf::Packet& p);
		bool receive();
		void handlePacket(sf::Packet& p);
		void bindCameraToControlledEntity();
		void sendHello();
		void displayMessage(std::string message);
		scene::ICameraSceneNode* getCamera();
};

#endif /* CLIENT_HPP_16_11_26_10_46_45 */
