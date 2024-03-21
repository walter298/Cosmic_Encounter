#include "NetworkUtil.h"

std::string nextDatum(StringIt& begin, StringIt end) {
	auto datumBreak = std::ranges::find(begin, end, DATUM_BREAK_CHR);
	std::string ret{ std::make_move_iterator(begin), std::make_move_iterator(datumBreak) };
	begin = std::next(datumBreak);
	return ret;
}

void Client::reconnect() {
	sys::error_code ec;
	while (true) {
		asio::connect(m_sock, m_endpoint, ec);
		if (!ec) {
			break;
		}
	}
}
asio::awaitable<void> Client::asyncReconnect() {
	sys::error_code ec;
	while (true) {
		co_await asio::async_connect(m_sock, m_endpoint, asio::redirect_error(asio::use_awaitable, ec));
		if (!ec) {
			break;
		}
	}
}

void Client::send(const std::string& msg) {
	m_msgBeingRcvd = msg;
	sys::error_code ec;
	while (true) {
		asio::write(m_sock, asio::buffer(m_msgBeingRcvd), ec);
		if (ec) {
			reconnect();
		} else {
			break;
		}
	}
}

asio::awaitable<void> Client::asyncSend(const std::string& msg) {
	m_msgBeingRcvd = msg;
	sys::error_code ec;
	while (true) {
		co_await asio::async_write(m_sock, asio::buffer(m_msgBeingRcvd), 
			asio::redirect_error(asio::use_awaitable, ec));
		if (ec) {
			reconnect();
		} else {
			break;
		}
	}
}

asio::awaitable<void> Server::acceptConnection() {
	while (true) {
		try {
			std::string buff;
			std::string name;

			auto sock = co_await m_acceptor.async_accept(asio::use_awaitable);
			
			co_await asio::async_read_until(sock, asio::dynamic_buffer(buff), MSG_END_DELIM, asio::use_awaitable);

			readMsg(buff, name);
			m_strand.post(
				[mSock = std::move(sock), mName = std::move(name), this]() mutable {
					auto cliEndpoint = mSock.remote_endpoint(); //store endpoint because we move from mSock next
					m_clients.emplace_back(std::move(mSock), cliEndpoint);
				}
			);
			break;
		}
		catch (std::exception& e) {
			std::cerr << e.what() << '\n';
		}
	}

	co_return;
}

Server::Server(tcp::endpoint&& endpoint)
	: m_strand{ m_context }, m_acceptor{ m_context, std::move(endpoint) }
{
	m_acceptor.listen();
}

Client& Server::getClient(size_t idx) {
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