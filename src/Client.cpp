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
#include "ShowAlienSystem.h"

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
	//music.play(9000000);

	joinGame(instance.renderer, sock, texMap, fontMap);
	
	//join lobby
	auto gameRenderData = runLobby(sock, instance.renderer, texMap, fontMap);
	
	TurnTakingDestiny turnTakingDestiny{ sock, instance.renderer, texMap, fontMap, gameRenderData.colorMap };
	NonTurnTakingDestiny nonTurnTakingDestiny{ sock, instance.renderer, texMap, fontMap, gameRenderData.colorMap };

	ShowAlienSystem showAlienSystem{ instance.renderer, texMap, fontMap };
	PlanetSelector planetSelector{ showAlienSystem };

	//get the cards and the turn order
	std::vector<Card> cards;
	std::vector<Color> turnOrder;
	sock.read(SocketHeader::CARDS_AND_TURN_ORDER, cards, turnOrder);

	size_t colorIdx = 0;
	while (true) {
		if (gameRenderData.pColor == turnOrder[colorIdx]) {
			auto color = turnTakingDestiny();
			Colonies colonies;
			sock.read(SocketHeader::COLONY_INFORMATION, colonies);
			auto colonyIdx = planetSelector(gameRenderData.renderer, color, colonies, gameRenderData.colorMap);
			sock.send(SocketHeader::CHOSEN_COLONY, colonyIdx);
		} else {
			nonTurnTakingDestiny();
		}
		exit(5555);
		//colorIdx = (colorIdx + 1) % turnOrder.size();
	}

	//show the game 
	showGameOverview(sock, instance.renderer, texMap, fontMap, gameRenderData);

	networkingThread.join();
}