#include "NetworkUtil.h"

std::string nextDatum(StringIt& begin, StringIt end) {
	auto datumBreak = std::ranges::find(begin, end, DATUM_BREAK_CHR);
	std::string ret{ begin, end };
	begin = std::next(datumBreak);
	return ret;
}

Socket::Socket(asio::io_context& context) 
	: m_sock { context }
{
}
Socket::Socket(tcp::socket&& sock) : m_sock{ std::move(sock) }
{
}
Socket::Socket(tcp::socket&& sock, ReconnCB&& whenDisconnected, ReconnCB&& asyncWhenDisconnected)
	: m_sock { std::move(sock) },
	m_whenDisconnected{ std::move(whenDisconnected) }, m_asyncWhenDisconnected{ std::move(asyncWhenDisconnected) }
{
}

void Socket::onDisconnected(ReconnCB&& cb) {
	m_whenDisconnected = std::move(cb);
}
void Socket::onAsyncDisconnected(ReconnCB&& cb) {
	m_asyncWhenDisconnected = std::move(cb);
}

void Socket::connect(const tcp::endpoint& endpoint) {
	tcp::resolver res{ m_sock.get_executor() };
	auto result = res.resolve(endpoint);
	asio::connect(m_sock, result);
}

void Socket::disconnect() {
	m_sock.close();
}

void Socket::send(const std::string& msg) {
	m_msgBeingRcvd = msg;
	sys::error_code ec;
	while (true) {
		asio::write(m_sock, asio::buffer(m_msgBeingRcvd), ec);
		if (ec) {
			m_whenDisconnected(ec, m_sock);
		} else {
			break;
		}
	}
}

asio::awaitable<void> Socket::asyncSend(const std::string& msg) {
	m_msgBeingRcvd = msg;
	sys::error_code ec;
	while (true) {
		co_await asio::async_write(m_sock, asio::buffer(m_msgBeingRcvd), 
			asio::redirect_error(asio::use_awaitable, ec));
		if (ec) {
			m_asyncWhenDisconnected(ec, m_sock);
		} else {
			break;
		}
	}
}

void Server::reconnect(sys::error_code ec, tcp::socket& socket) {
	while (true) {
		auto newSock = m_acceptor.accept(ec);
		if (newSock.remote_endpoint() == socket.remote_endpoint()) {
			socket = std::move(newSock);
			break;
		}
	}
}

void Server::asyncReconnect(sys::error_code ec, tcp::socket& sock) {
	m_strand.dispatch([&, this] { reconnect(ec, sock); });
}

asio::awaitable<void> Server::acceptConnection() {
	sys::error_code ec;

	auto sock = co_await m_acceptor.async_accept(asio::redirect_error(asio::use_awaitable, ec));
	if (ec) {
		std::cerr << ec << '\n';
	}
	m_strand.post(
		[mSock = std::move(sock), this]() mutable {
			m_clients.emplace_back(std::move(mSock),
				[this](auto ec, auto& sock) { reconnect(ec, sock); },
				[this](auto ec, auto& sock) { asyncReconnect(ec, sock); }
			);
		}
	);
}

Server::Server(const tcp::endpoint& endpoint)
	: m_strand{ m_context }, m_acceptor{ m_context, endpoint }
{
	m_acceptor.listen();
}

Socket& Server::getClient(size_t idx) {
	return m_clients[idx];
}

void Server::acceptConnections(size_t connC) {
	m_clients.reserve(connC);
	for (size_t i = 0; i < connC; i++) {
		asio::co_spawn(m_context, acceptConnection(), asio::detached);
	}
	m_context.run();
}

void Server::broadcast(const std::string& msg) {
	for (auto& client : m_clients) {
		asio::co_spawn(m_context, client.asyncSend(msg), asio::detached);
	}
	m_context.run();
}