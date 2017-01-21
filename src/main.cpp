#include <cstring>
#include <main.hpp>
#include <controller.hpp>
#include <world.hpp>
#include <server.hpp>
#include <client.hpp>

int main(int argc, char* argv[]) {
	if(argc != 2) {
		return 1;
	}
	else if(!strcmp(argv[1], "-s")) {
		//TODO move irrDev creation to serverApp ... irrDev has to be created before serverApp .. probably
		// maybe it makes sense to create some parts of application later - create world after ... 
		irr::SIrrlichtCreationParameters params;
		params.DriverType = video::E_DRIVER_TYPE::EDT_NULL;
		IrrlichtDevice* device = createDeviceEx(params);
		ServerApplication s(device);
		if(!s.listen(5555)) {
			cerr << "Cannot listen on given port.\n";
			return 1;
		}
		s.run();
		device->drop();
	}
	else if(!strcmp(argv[1], "-c")) {
		ClientApplication c;
		if(!c.connect("localhost", 5555)) {
			cerr << "Failed to connect to the server.\n";
			//return 1;
		}
		c.run();
	}
	return 0;
}
