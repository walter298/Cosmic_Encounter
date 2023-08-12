#include <sstream>

#include "Game.h"
#include "ClientSide.h"

#include "novalis/Instance.h"

#undef main

void host(const tcp::endpoint& endpoint, size_t pCount) {
	asio::io_context context;
	Game game{ context, endpoint };

	std::error_code ec;
	game.waitForPlayers(pCount, ec);
	if (ec) {
		std::cerr << ec.message() << std::endl;
		return;
	}
	game.setup();
}

int main(int argc, char* argv[]) {
	constexpr asio::ip::port_type port = 17356;

	std::error_code ec;
	tcp::endpoint endpoint{ asio::ip::address_v4::from_string("192.168.7.250", ec), port };

	if (ec) {
		std::cerr << "Error: " << ec.message() << std::endl;
		return -1;
	}

	std::jthread serverThread { host, endpoint, 1 };

	std::cout << "Enter your name: ";
	std::string name;
	std::cin >> name;
	play(endpoint, name);

	serverThread.join();
}