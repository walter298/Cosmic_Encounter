#include "Client.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <fstream>
#include <ranges>
#include <vector>

#include "novalis/Instance.h"
#include "novalis/Sound.h"

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
	auto alienTexObjs = runLobby(sock, instance.renderer, texMap, fontMap);
	
	//show the game 
	showGameOverview(sock, instance.renderer, texMap, fontMap);

	networkingThread.join();
}