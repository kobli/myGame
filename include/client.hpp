#ifndef CLIENT_HPP_16_11_26_10_46_45
#define CLIENT_HPP_16_11_26_10_46_45 
#include <SFML/Network.hpp>
#include "main.hpp"
#include "world.hpp"
#include "system.hpp"
#include "keyValueStore.hpp"
#include "timedFilter.hpp"
#include "progressBar.hpp"

class Animator: public Observer<EntityEvent>
{
	public:
		Animator(scene::ISceneManager* smgr = nullptr, function<Entity*(u32)> entityResolver = [](u32){ return nullptr; }
				, function<vec3f(u32)> entityVelocityGetter = function<vec3f(u32)>());
		void setEntityResolver(std::function<Entity*(u32 ID)> entityResolver);
		void setEntityVelocityGetter(std::function<vec3f(u32 ID)> entityVelocityGetter);
		void setSceneManager(scene::ISceneManager* smgr);

	private:
		scene::ISceneManager* _smgr;
		function<Entity*(u32)> _entityResolver;
		std::function<vec3f(u32 ID)> _velGetter;
		void onMsg(const EntityEvent& m);

};

////////////////////////////////////////////////////////////

class ClientApplication: private Observer<EntityEvent>
{
	public:
		ClientApplication();
		bool connect(string host, unsigned short port);
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
		KeyValueStore _sharedRegistry;
		float _cameraElevation;
		float _cameraYAngle;
		TimedFilter<float> _yAngleSetCommandFilter;
		gui::ProgressBar* _healthBar;
		gui::ProgressBar* _castingIndicator;
		gui::IGUIStaticText* _spellInHandsInfo;


		void onMsg(const EntityEvent& m) override;
		void commandHandler(Command& c);
		void sendCommand(Command& c);
		void sendPacket(sf::Packet& p);
		bool receive();
		void handlePacket(sf::Packet& p);
		void bindCameraToControlledEntity();
		void updateCastingIndicator(float timeDelta);
};

#endif /* CLIENT_HPP_16_11_26_10_46_45 */
