#include "Client.h"

ConnectionState Client::joinGame(asio::io_context& context, const tcp::endpoint& endpoint) {
	tcp::resolver resolver{ context };

	std::error_code ec;

	//connect to server
	handler.socket.connect(endpoint, ec);
	if (ec) {
		std::cerr << "Error connecting to server: " << ec.message() << std::endl;
		return ConnectionState::FailedToConnect;
	}

	auto joinRes = handler.recv(ec);
	if (ec) {
		std::cerr << "Error joining game: " << ec.message() << std::endl;;
		return ConnectionState::FailedToConnect;
	}

	//tell the server our name
	handler.send(name, ec);
	if (ec) {
		std::cerr << "Error sending name data: " << ec.message() << std::endl;
		return ConnectionState::FailedToConnect;
	}

	//recieve card data
	auto playerRes = handler.recv(ec);
	if (ec) {
		std::cerr << "Error receiving player data: " << ec.message() << std::endl;
		return ConnectionState::FailedToConnect;
	}

	p.hand = playerRes.value();
	
	return ConnectionState::Connected;
}

