#include "Client.h"
#include "Server.h"

#include "novalis/editor/EditorApp.h"
#include "novalis/Instance.h"

#include "JoinGame.h"
#include "GameTests.h"

#include <thread>

#undef main

/*
* argv[1] = port 
* argv[2] = ipv4 address
* player count
*/
int main(int argc, char** argv) {
	/*try {
		testPlanetSelector();
	} catch (nlohmann::json::exception& e) {
		std::println("{}", e.what());
	}*/
	nv::editor::runEditors();

	//testAlliance();

	/*tcp::endpoint ep{ ip::make_address_v4("192.168.7.250"), 5555 };
	std::jthread serverThread{ host, 2, std::ref(ep) };
	
	play();

	serverThread.join();*/
}
