#include "ClientSide.h"

void play(const tcp::endpoint& endpoint) {
	asio::io_context context;
	Socket cli{ context };
	
	std::cout << "Connecting to server...\n";

	try {
		cli.connect(endpoint);
	} catch (std::exception& e) {
		std::cerr << e.what();
		return;
	}

	
}