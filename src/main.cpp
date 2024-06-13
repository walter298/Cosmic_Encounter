#include "ClientSide.h"
#include "ServerSide.h"

#include "novalis/EditorApp.h"

#undef main

/*
* argv[1] = port 
* argv[2] = ipv4 address
* argv[3] = run_server or play
* argv[4] = if argv[2] == run_server, then player count
*/
int main(int argc, char** argv) {
	nv::editor::runEditors();

	/*tcp::endpoint endpoint{
		ip::address_v4::from_string(argv[1]),
		static_cast<ip::port_type>(std::stoi(argv[2]))
	};

	play(std::move(endpoint));*/

	//if (strcmp(argv[3], "run_server\0")) {
	//	//host(std::atoi(argv[4]), endpoint);
	//} else {
	//	play(std::move(endpoint));
	//}
}