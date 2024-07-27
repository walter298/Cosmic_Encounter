#include "Server.h"

#include <iostream> //cin
#include <print>

#include "Game.h"
#include "Lobby.h"

namespace ranges = std::ranges;
namespace views = std::views;

template<typename... Args>
static void broadcast(Players& players, Args&&... args) {
	std::tuple argsTuple{ std::forward<Args>(args)... };
	for (auto& player : players) {
		std::apply([&](const auto&... tupleArgs) { 
			player.sock.send(tupleArgs...); 
		}, argsTuple);
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
			defensivePlayer = &*ranges::find_if(gameState.players, [&](const auto& player) { 
				return player.color == turnTaker.color; 
			});
			break;
		} else if (acceptedOwnColorBeingDrawn(turnTaker, colonyIdx)) {
			break;
		}
	}
	return { defensivePlayer, colonyIdx };
}

static Players acceptConnections(size_t pCount, tcp::acceptor& acceptor, std::random_device& rbg) {
	std::println("Waiting for players to join");

	assert(pCount < 6);

	std::vector<Player> players;

	//Players players;
	players.reserve(pCount);

	std::array availableColors = { Red, Green, Blue, Black, Purple };
	std::array availableAliens = { Pacifist, Virus };

	ranges::shuffle(availableColors, rbg);
	ranges::shuffle(availableAliens, rbg);

	//accept connections
	for (size_t i = 0; i < pCount; i++) {
		auto& player = players.emplace_back(Socket{ acceptor.accept() }, availableColors[i]);

		//send each player game data: their color, their alien, turn order
		player.sock.send(availableColors[i], availableAliens[i], 
			ranges::subrange(availableColors.begin(), availableColors.begin() + pCount));
	}

	return players;
}

static std::vector<Color> loadDestinyDeck(const Players& players) {
	std::vector<Color> colors{ players.size() * 3 };
	size_t playerIdx = 0;

	ranges::generate(colors, [&]() {
		auto color = players[playerIdx].color;
		playerIdx += ((playerIdx + 1) % players.size());
		return color;
	});

	return colors;
}

void host(size_t pCount, const tcp::endpoint& endpoint) {
	asio::io_context ctx;
	
	//run networking asynchronously
	std::jthread ctxThread{ [&] { 
		asio::io_context::work work{ ctx };
		ctx.run(); 
	}};

	tcp::acceptor acceptor{ ctx, endpoint };
	
	GameState gameState;
	gameState.players = acceptConnections(pCount, acceptor, gameState.rbg);

	std::println("Press enter to start the game");
	std::cin.get();

	//tell everyone the game is starting
	broadcast(gameState.players, StartingGame);

	//give each player 8 cards
	for (auto& player : gameState.players) {
		gameState.deck.draw(player.hand, 8);
		player.sock.send(player.hand);
	}

	gameState.destinyDeck = loadDestinyDeck(gameState.players);

	size_t turnTakerIdx = 0;

	//game loop
	while (true) {
		auto& turnTaker = gameState.players[turnTakerIdx];
		auto defense = getDefense(turnTaker, gameState);
	}
}