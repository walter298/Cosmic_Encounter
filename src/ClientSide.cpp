#include "ClientSide.h"

void play() {
	std::string ipAddr;
	ip::port_type port = 0;

	std::cout << "Enter host IP address: ";
	std::cin >> ipAddr;
	std::cout << "Enter host port: ";
	std::cin >> port;

	asio::io_context context;
	Socket cli{ context };

}