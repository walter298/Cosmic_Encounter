#include "Server.h"

#include <iostream> //cin
#include <print>

#include "Game.h"

namespace ranges = std::ranges;
namespace views = std::views;

template<typename... Args>
static void broadcast(Players& players, Args&... args) {
	for (auto& player : players) {
		player.sock.send(args...);
	}
}

static bool acceptedOwnColorBeingDrawn(Player& turnTaker, size_t& colonyIdx) {
	if (std::ranges::any_of(turnTaker.colonies,
		[](const auto& colony) { return colony.hasEnemyShips; })) {
		bool accepted = false;
		Color color{};
		turnTaker.sock.read(accepted, color, colonyIdx);
		return accepted;
	} else {
		return false;
	}
}

struct Defense {
	Player* player;
	size_t colonyIdx = 0;
};

static Defense getDefense(Player& turnTaker, GameState& gameState) {
	Player* defensivePlayer = nullptr;
	size_t colonyIdx = 0;

	while (true) {
		auto colorDrawn = gameState.destinyDeck.discardTop();
		broadcast(gameState.players, colorDrawn);

		if (colorDrawn != turnTaker.color) { //player does not have a choice to keep drawing if another color is drawn
			defensivePlayer = &gameState.players[colorDrawn];
			break;
		} else if (acceptedOwnColorBeingDrawn(turnTaker, colonyIdx)) {
			break;
		}
	}
	return { defensivePlayer, colonyIdx };
}

static Players acceptConnections(size_t pCount, tcp::acceptor& acceptor, std::mt19937& rbg) {
	assert(pCount < 6);

	Players players;
	players.reserve(pCount);

	std::array availableColors = { Red, Green, Blue, Black, Purple };
	std::array availableAliens = { Pacifist, Virus };

	ranges::shuffle(availableColors, rbg);
	ranges::shuffle(availableAliens, rbg);

	size_t colorAlienIdx = 0;

	//accept connections
	for (size_t i = 0; i < pCount; i++) {
		auto& player = players.emplace_back(acceptor.accept(), availableColors[colorAlienIdx]);
		player.sock.send(availableColors[colorAlienIdx], availableAliens[colorAlienIdx]);
		colorAlienIdx++;
	}

	std::println("Hit any key to start game");
	std::cin.get();
	
	return players;
}

void host(size_t pCount, const tcp::endpoint& endpoint) {
	asio::io_context ctx;
	tcp::acceptor acceptor{ ctx, endpoint };
	
	GameState gameState;
	gameState.players = acceptConnections(pCount, acceptor, gameState.rbg);

	size_t turnTakerIdx = 0;

	//game loop
	/*while (true) {
		auto& turnTaker = gameState.players[turnTakerIdx];
		auto defense = getDefense(turnTaker, gameState);
	}*/
	std::cin.get();
}