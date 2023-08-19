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

	//tell server our name is m_name
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

	std::cout << "Getting Cards\n";
	json parsedJson = playerRes.value();
	try {
		p.hand = parsedJson;
	} catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
	std::cout << "Returning connected\n";

	return ConnectionState::Connected;
}

