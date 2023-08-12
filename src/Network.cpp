#include "Network.h"

MessageHandler::MessageHandler(tcp::socket&& socket)
	: m_socket{ std::move(socket) } {}

MessageHandler::MessageHandler(asio::io_context& context)
	: m_socket{ context } {}

tcp::socket* MessageHandler::operator->() {
	return &m_socket;
}

void MessageHandler::send(json msg, std::error_code& ec) {
	std::string serializedJson = msg.dump() + '!';

	if (serializedJson.size() < maxMsgSize) {
		asio::write(m_socket, asio::buffer(serializedJson, serializedJson.size()), ec);
		if (ec) {
			return;
		} 
	} else {
		ec = std::make_error_code(std::errc::value_too_large);
	}
}

std::optional<json> MessageHandler::recv(std::error_code& ec)
{
	asio::streambuf streambuf;
	streambuf.prepare(maxMsgSize);

	asio::streambuf buf{ maxMsgSize };

	asio::read_until(m_socket, buf, '!', ec);
	
	if (ec) {
		return std::nullopt;
	}

	//store the buffer contents in a string
	std::string msgStr{ asio::buffer_cast<const char*>(buf.data()) };
	msgStr.pop_back(); //get rid of exclamation

	//parse the json
	json parsedJson = json::parse(msgStr);

	if (parsedJson.is_discarded()) {
		ec = std::make_error_code(std::errc::bad_message);
		return std::nullopt;
	} else {
		return parsedJson;
	}
}

Server::Server(asio::io_context& context, tcp::endpoint endpoint)
	: m_context{ context }, m_acceptor{ context, endpoint.protocol() },
	m_endpoint{ endpoint } {}

const tcp::endpoint& Server::endpoint() const noexcept {
	return m_endpoint;
}

void Server::listen(std::function<void(tcp::socket&&)> socketAction, std::function<bool()> stopPred,
	std::error_code& ec)
{
	m_acceptor.bind(m_endpoint, ec);
	if (ec) {
		return;
	}
	m_acceptor.listen();

	while (!stopPred()) {
		socketAction(m_acceptor.accept());
	}
	m_acceptor.close();
}