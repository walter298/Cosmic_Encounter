#pragma once

#include <optional>
#include <iostream>
#include <expected>
#include <chrono>

#include <asio.hpp>
#include <nlohmann/json.hpp>

using asio::ip::tcp;
using nlohmann::json;

using namespace std::chrono;

enum class ConnectionState {
	NotConnecting,
	Connecting,
	FailedToConnect,
	Connected
};

class MessageHandler {
private:
	static constexpr size_t maxMsgSize = 8192;
	static constexpr seconds maxMsgWait = 20s;

	tcp::socket m_socket;
public:
	explicit MessageHandler(tcp::socket&& socket);
	explicit MessageHandler(asio::io_context& context);

	tcp::socket* operator->();

	void send(json msg, std::error_code& ec);

	std::optional<json> recv(std::error_code& ec);
};

class Server {
private:
	asio::io_context& m_context;
	tcp::endpoint m_endpoint;
	tcp::acceptor m_acceptor;
public:
	Server(asio::io_context& context, tcp::endpoint endpoint);

	const tcp::endpoint& endpoint() const noexcept;
	
	void listen(std::function<void(tcp::socket&&)> socketAction, 
		std::function<bool()> stopPred, std::error_code& ec);
};