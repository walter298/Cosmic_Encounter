#include "Client.h"
#include "Server.h"

#include "novalis/EditorApp.h"
#include "novalis/Instance.h"

#include "JoinGame.h"

#include <thread>

#undef main

/*
* argv[1] = port 
* argv[2] = ipv4 address
* player count
*/
int main(int argc, char** argv) {
	//nv::editor::runEditors();

	tcp::endpoint ep{ ip::make_address_v4("192.168.7.250"), 5555 };
	std::jthread serverThread{ host, 1, std::ref(ep) };
	
	play();

	serverThread.join();
}