#include "ServerSide.h"

struct Defense {
	Player* player;
	size_t colonyIdx = 0;
};

static Defense getDefense(Player& turnTaker, GameState& gameState, Server& svr) {
	Player* defensivePlayer = nullptr;
	size_t colonyIdx = 0;

	while (true) {
		auto colorDrawn = gameState.destinyDeck.discardTop();
		svr.broadcast(writeMsg(colorDrawn));

		if (colorDrawn != turnTaker.color) {
			defensivePlayer = &gameState.players[colorDrawn];
			turnTaker.cli.send(writeMsg(defensivePlayer->color));
			turnTaker.cli.read(colonyIdx);
			break;
		}
		else if (std::ranges::any_of(turnTaker.colonies,
			[](const auto& colony) { return colony.hasEnemyShips; })) {
			bool accepted{};
			Color color{};
			turnTaker.cli.read(accepted, color, colonyIdx);
			if (accepted) {
				defensivePlayer = &gameState.players[color];
				break;
			}
		}
	}
	return { defensivePlayer, colonyIdx };
}

void host(size_t pCount, tcp::endpoint&& endpoint) {
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
		auto defense = getDefense(turnTaker, gameState, svr);
	}
}