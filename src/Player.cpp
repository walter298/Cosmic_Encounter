#include "Player.h"

Player::Player(MessageHandler&& msgHandler, std::string name)
	: msgHandler{ std::move(msgHandler) }, name{ std::move(name) }
{}

Player::Player(asio::io_context& context, std::string name)
	: msgHandler(context), name(std::move(name)) {}

MessageHandler* Player::operator->() {
	return &msgHandler;
}

ConnectionState Player::joinGame(asio::io_context& context, const tcp::endpoint& endpoint) {
	tcp::resolver resolver{ context };
	MessageHandler handler{ context };
	
	std::error_code ec;
	
	//connect to server
	handler->connect(endpoint, ec);
	if (ec) {
		std::cerr << "Error connecting to server: " << ec.message() << std::endl;
		return ConnectionState::FailedToConnect;
	}
	
	auto joinResult = handler.recv(ec);
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
	
	auto playerResult = handler.recv(ec);
	if (ec) {
		std::cerr << "Error receiving player data: " << ec.message() << std::endl;
		return ConnectionState::FailedToConnect;
	}

	json parsedJson = playerResult.value();
	hand = parsedJson.front().get<Cards>();

	return ConnectionState::Connected;
}