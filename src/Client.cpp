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

struct PlayerRenderData {
	nv::TextureObject alien;
	nv::Sprite planets;
};

static PlayerRenderData getPlayerRenderData(SDL_Renderer* renderer, nv::TextureMap& texMap, Alien alien, Color color) {
	auto alienObj = [&]() {
		std::string path;
		switch (alien) {
		case Pacifist:
			path = nv::relativePath("Cosmic_Encounter/game_assets/texture_objects/pacifist.nv_texture_object");
			break;
		case Virus:
			path = nv::relativePath("Cosmic_Encounter/game_assets/texture_objects/virus.nv_texture_object");
			break;
		case Zombie:
			path = nv::relativePath("Cosmic_Encounter/game_assets/texture_objects/zombie.nv_texture_object");
			break;
		case Laser:
			path = nv::relativePath("Cosmic_Encounter/game_assets/texture_objects/laser.nv_texture_object");
			break;
		case Oracle:
			path = nv::relativePath("Cosmic_Encounter/game_assets/texture_objects/oracle.nv_texture_object");
			break;
		}
		std::ifstream jsonFile{ path };
		assert(jsonFile.is_open());
		return nv::TextureObject{ renderer, nlohmann::json::parse(jsonFile), texMap };
	}();
	
	std::ifstream jsonFile{ nv::relativePath("Cosmic_Encounter/game_assets/sprites/planets.nv_sprite") };
	assert(jsonFile.is_open());
	nv::Sprite planets{ renderer, nlohmann::json::parse(jsonFile), texMap };

	return { std::move(alienObj), std::move(planets) };
}

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
	
	//server spits out our alien and color and player count
	Color color; 
	Alien alien;
	std::vector<Color> turnOrder;
	sock.read(color, alien, turnOrder);

	auto [alienObj, planetsSprite] = getPlayerRenderData(instance.renderer, texMap, alien, color);

	//join lobby
	auto alienTexObjs = runLobby(sock, turnOrder.size(), alienObj, instance.renderer, texMap, fontMap);
	
	//show the game 
	showGameOverview(sock, instance.renderer, alienTexObjs, texMap, fontMap, turnOrder, color);
	
	/*auto cardSprites = loadCardSprites(instance.renderer, texMap);
	ranges::sort(cardSprites, [](const auto& card1, const auto& card2) {
		return card1.getName() < card2.getName();
	});

	*/

	networkingThread.join();
}