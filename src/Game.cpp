#include "Game.h"

std::string toString(Card card) {
	return toString(static_cast<int>(card.type)) + '!' + toString(card.attackValue) + '!';
}

//[from, to] is inclusive
template<std::integral Integral>
static Integral randomNum(Integral from, Integral to) {
	static std::random_device dev;
	static std::mt19937 gen{ dev };
	std::uniform_int_distribution dist{ from, to };
	return dist(gen());
}

static Player& getDefensivePlayer(Player& turnTaker, GameState& gameState) {
	while (true) {
		auto colorDrawn = gameState.destinyDeck.discardTop();
		if (colorDrawn != turnTaker.color) {
			return gameState.players[colorDrawn];
		} else {
			//check if there are ships occupying home system

		}
	}
}

void play(size_t pCount, tcp::endpoint&& endpoint) {
	Server svr{ std::move(endpoint) };
	svr.acceptConnections(pCount);

	GameState gameState;

	auto& players = gameState.players;
	gameState.players.reserve(pCount);
	for (size_t i = 0; i < pCount; i++) {
		gameState.players.emplace_back(svr.getClient(i));
	}

	size_t turnTakerIdx = 0;

	//game loop
	while (true) {
		auto& turnTaker = players[turnTakerIdx];
		auto& defensivePlayer = getDefensivePlayer(turnTaker, gameState);
	}
}