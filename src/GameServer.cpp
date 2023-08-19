#include "GameServer.h"

void GameServer::addPlayer(tcp::socket&& socket, const Color& color) {
	MessageHandler handler{ std::move(socket) };
	std::error_code ec;
	handler.send("you are connected", ec);
	
	if (!ec) {
		auto nameResult = handler.recv(ec);
		if (ec) {
			std::cerr << "Error recieving message: " << ec.message() << std::endl;
			return;
		}
		std::string name = nameResult->front();
		m_players.emplace(color, Client{ name, std::move(handler) });
		std::cout << nameResult.value() << " has joined the game!\n";
	} else {
		std::cerr << "Error sending message: " << ec.message() << std::endl;
	}
}

void GameServer::setup() {
	m_deck.reserve(72);
	m_deck = makeDeck();

	//shuffle
	std::random_device rd;
	std::mt19937 gen(rd());
	rg::shuffle(m_deck, gen);

	//give each player a hand
	for (auto& [color, cli] : m_players) {
		moveTopContentsOut(cli.p.hand, m_deck, 8);
		std::error_code ec;
		cli.handler.send(cli.p.hand, ec);
		if (ec) {
			std::cerr << cli.name << " disconnected.\n";
		}
	}
}

GameServer::GameServer(asio::io_context& context, tcp::endpoint endpoint)
	: m_server{ context, std::move(endpoint) }
{
}

void GameServer::waitForPlayers(size_t pCount, std::error_code& ec)
{
	std::deque availableColors = {
		Color::Red,
		Color::Green,
		Color::Blue,
		Color::Yellow,
		Color::Purple,
		Color::Black
	};

	m_server.listen(
		[this, &availableColors](tcp::socket&& socket) { 
			addPlayer(std::move(socket), availableColors.front()); 
			availableColors.pop_front();
		}, 
		[this, pCount] { return m_players.size() == pCount; }, ec
	);

	if (!ec) {
		std::cout << "All players have connected! Starting the game...\n";
	}
}