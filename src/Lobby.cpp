#include "Lobby.h"

#include <algorithm>
#include <mutex>

#include "novalis/Scene.h"

#include "Game.h"

static asio::awaitable<void> recvTexPaths(Socket& sock, size_t pCount, std::vector<std::string>& texPathsToLoad, bool& gameStarting, std::mutex& mutex) {
	LobbyUpdate lobbyUpdate = None;

	bool readingBytes = true;
	while (readingBytes) {
		co_await sock.asyncRead(lobbyUpdate);

		std::scoped_lock lock{ mutex };
		switch (lobbyUpdate) {
		case StartingGame:
			gameStarting = true;
			readingBytes = false;
			break;
		case PacifistJoining:
			texPathsToLoad.push_back("Cosmic_Encounter/game_assets/texture_objects/pacifist.nv_texture_object");
			break;
		case VirusJoining:
			texPathsToLoad.push_back("Cosmic_Encounter/game_assets/texture_objects/virus.nv_texture_object");
			break;
		}
	}
}

static void checkForLobbyUpdate(nv::Scene& lobby, int& alienX, const int alienY, std::vector<std::string> texPathsToLoad, bool& gameStarting, std::mutex& mutex) {
	std::scoped_lock lock{ mutex };
	if (gameStarting) {
		lobby.running = false;
		return;
	}
	auto loadAlien = [&](std::string_view relativePath) {
		nv::TextureObject alien{
			lobby.renderer,
			nv::parseFile(nv::relativePath(relativePath)),
			lobby.texMap
		};
		alien.setPos(alienX, alienY);
		alienX += alien.getSize().x;
		lobby.textures[0].push_back(std::move(alien));
	};
	if (!texPathsToLoad.empty()) {
		loadAlien(texPathsToLoad.back());
		texPathsToLoad.pop_back();
	}
}

static std::vector<nv::TextureObject> getAlienTexObjs(std::vector<nv::TextureObject>& texObjs) {
	namespace ranges = std::ranges;

	std::vector<nv::TextureObject> alienObjs;
	alienObjs.reserve(texObjs.size() - 1);
	ranges::move(ranges::subrange(texObjs.begin() + 1, texObjs.end()), std::back_inserter(alienObjs));

	return alienObjs;
}

std::vector<nv::TextureObject> runLobby(Socket& sock, size_t pCount, nv::TextureObject& alien, SDL_Renderer* renderer, nv::TextureMap& texMap, nv::FontMap& fontMap) {
	nv::Scene lobby{ nv::relativePath("Cosmic_Encounter/game_assets/scenes/lobby.nv_scene"), renderer, texMap, fontMap };
	
	int alienX = 10;
	const int alienY = 500;

	alien.setPos(alienX, alienY);
	lobby.textures[0].push_back(alien);

	std::vector<std::string> texPathsToLoad;
	bool gameStarting = false;
	std::mutex mutex;

	//asynchrously read alien texture paths off the network
	asio::co_spawn(sock.getExecutor(), recvTexPaths(sock, pCount, texPathsToLoad, gameStarting, mutex), asio::detached);

	//periodically check if server has sent over alien texture paths to load
	lobby.eventHandler.addPeriodicEvent([&] { checkForLobbyUpdate(lobby, alienX, alienY, texPathsToLoad, gameStarting, mutex); });
	
	lobby();
	
	return getAlienTexObjs(lobby.textures[0]);
}
