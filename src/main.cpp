#include <iostream>

#include "Game.h"

using namespace std;

/*
* if argv[1] == run_server 
* argv[2] = player count
* argv[3] = ipv6 address
* argv[4] = port
* else if argv[1] == play_game
* argv[2] = ipv6 address
* argv[3] = port
*/
int main(int argc, char** argv) {
	if (strcmp(argv[1], "run_server ")) {
		play(std::atoi(argv[2]), 
			tcp::endpoint{ 
				ip::address_v4::from_string(argv[3]), 
				static_cast<ip::port_type>(std::stoi(argv[4])) 
			}
		);
	}
}