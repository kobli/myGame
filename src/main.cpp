#include <cstring>
#include "main.hpp"
#include "controller.hpp"
#include "world.hpp"
#include "server.hpp"
#include "client.hpp"

bool hasCmdOption(const int argc, char* argv[], const std::string& option, std::string* outArg = nullptr)
{
	for(int i = 0; i < argc; i++) {
		if(std::string(argv[i]) == option) {
			if(outArg != nullptr) {
				if(i+1 < argc && argv[i+1][0] != '-')
					*outArg = argv[i+1];
			}
			return true;
		}
	}
	return false;
}

int main(int argc, char* argv[]) {
	auto getCmdOption = [&](const std::string& option, std::string* outArg = nullptr) {
		return hasCmdOption(argc-1, argv+1, option, outArg);
	};
	std::string port = "5555";
	getCmdOption("-p", &port);
	if(getCmdOption("-s")) {
		irr::SIrrlichtCreationParameters params;
		params.DriverType = video::E_DRIVER_TYPE::EDT_NULL;
		IrrlichtDevice* device = createDeviceEx(params);
		ServerApplication s(device);
		if(!s.listen(std::stoi(port))) {
			cerr << "Cannot listen on port " << port << ".\n";
			return 1;
		}
		std::cout << "listening on " << port << std::endl;
		s.run();
		device->drop();
	}
	else if(getCmdOption("-c")) {
		ClientApplication c;
		std::string addr = "localhost";
		getCmdOption("-a", &addr);
		if(!c.connect(addr, std::stoi(port))) {
			cerr << "Failed to connect to the server.\n";
			return 1;
		}
		std::cout << "connected\n";
		c.run();
	}
	else {
		std::cout << "options:\n"
			<< "\t -c\tclient mode\n"
			<< "\t -s\tserver mode\n"
			<< "\t -p\tport\n"
			<< "\t -a\taddress\n";
	}
	return 0;
}
