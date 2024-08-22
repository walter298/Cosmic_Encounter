#include "Client.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <fstream>
#include <ranges>
#include <vector>

#include "novalis/Instance.h"
#include "novalis/Sound.h"

#include "Destiny.h"
#include "Game.h"
#include "GameOverviewUI.h"
#include "JoinGame.h"
#include "Lobby.h"

namespace ranges = std::ranges;
namespace views  = std::views;

void play() {
	asio::io_context context;

	std::jthread networkingThread{ [&] {
		asio::io_context::work dummy{ context };
		context.run();
	}};

	Socket sock{ context };

	nv::Instance instance{ "Cosmic Encounter" };
	
	nv::TextureMap texMap;
	nv::FontMap fontMap;

	nv::Music music{ nv::relativePath("Cosmic_Encounter/game_assets/music/mars.wav") };
	music.play(9000000);

	joinGame(instance.renderer, sock, texMap, fontMap);
	
	//join lobby
	auto gameRenderData = runLobby(sock, instance.renderer, texMap, fontMap);
	
	TurnTakingDestiny turnTakingDestiny{ sock, instance.renderer, texMap, fontMap, gameRenderData.colorMap };
	NonTurnTakingDestiny nonTurnTakingDestiny{ sock, instance.renderer, texMap, fontMap, gameRenderData.colorMap };

	//get the cards and the turn order
	std::vector<Card> cards;
	std::vector<Color> turnOrder;
	sock.read(cards, turnOrder);

	size_t colorIdx = 0;
	while (true) {
		if (gameRenderData.pColor == turnOrder[colorIdx]) {
			turnTakingDestiny();
		} else {
			nonTurnTakingDestiny();
		}
		colorIdx = (colorIdx + 1) % turnOrder.size();
	}

	//show the game 
	showGameOverview(sock, instance.renderer, texMap, fontMap, gameRenderData);

	networkingThread.join();
}