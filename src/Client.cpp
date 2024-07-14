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
#include "JoinGame.h"
#include "Lobby.h"

namespace ranges = std::ranges;
namespace views  = std::views;

static std::vector<nv::TextureObject> loadCardSprites(SDL_Renderer* renderer, nv::TextureMap& texMap) {
	std::ifstream file{ nv::relativePath("Cosmic_Encounter/game_assets/scenes/card_objects.json") };
	auto json = nv::json::parse(file);

	std::vector<nv::TextureObject> ret;
	ret.reserve(20);

	for (const auto& texObjJson : json) {
		ret.emplace_back(renderer, json, texMap);
	}

	return ret;
}

struct PlayerPersona {
	nv::TextureObject alien;
	nv::Sprite planets;
};

static PlayerPersona getPlayerPersona(SDL_Renderer* renderer, nv::TextureMap& texMap, Alien alien, Color color) {
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
	Socket sock{ context };

	nv::Instance instance{ "Cosmic Encounter" };
	nv::TextureMap texMap;
	nv::FontMap fontMap;

	nv::Music music{ nv::relativePath("Cosmic_Encounter/game_assets/music/mars.wav") };
	music.play(9000000);

	joinGame(instance.renderer, sock, texMap, fontMap);
	
	//server spits out our alien and color
	Color color; Alien alien;
	sock.read(color, alien);

	auto [alienObj, planetsSprite] = getPlayerPersona(instance.renderer, texMap, alien, color);

	//join lobby
	runLobby(sock, alienObj, instance.renderer, texMap, fontMap);


	/*auto cardSprites = loadCardSprites(instance.renderer, texMap);
	ranges::sort(cardSprites, [](const auto& card1, const auto& card2) {
		return card1.getName() < card2.getName();
	});

	*/
}