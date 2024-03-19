#include "NetworkUtil.h"

std::string nextDatum(StringIt& begin, StringIt end) {
	auto datumBreak = std::ranges::find(begin, end, DATUM_BREAK_CHR);
	std::string ret{ std::make_move_iterator(begin), std::make_move_iterator(datumBreak) };
	begin = std::next(datumBreak);
	return ret;
}

asio::awaitable<void> Server::reconnectPlayer(Client& cli) {
	/*try {
		tcp::socket sock{ m_context };
		co_await asio::async_connect(sock, cli.endpoint, asio::use_awaitable);
		cli.sock = std::move(sock);
	}
	catch (std::exception& e) {
		std::cerr << e.what() << '\n';
	}*/

	co_return;
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

	//waitForPlayersToJoin(pCount);

	//sendToPlayers(
	//	[color = 0, this](Player& player) mutable {
	//		m_gameState.deck.draw(player.hand, 8); //give player 8 cards
	//		auto msg = writeMsg(color, player.hand); //send player their color (idx) and hand
	//		color++;
	//		return msg;
	//	}
	//);
}

Client& Server::getClient(size_t idx) {
	return m_clients[idx];
}

void Server::acceptConnections(int connC) {
	m_clients.reserve(static_cast<size_t>(connC));
	for (int i = 0; i < connC; i++) {
		asio::co_spawn(m_context, acceptConnection(), asio::detached);
	}
	m_context.run();
}
