#include "Lobby.h"

#include <boost/lockfree/queue.hpp>

#include "novalis/Scene.h"

#include "Game.h"

std::mutex mutex;
std::condition_variable cv;
bool isLobbyUpdate = false;
bool hasLoadedAlien = false;
LobbyUpdate lobbyUpdate = None;

using TexturePaths = boost::lockfree::queue<std::string>;

static asio::awaitable<void> checkForLobbyUpdates(Socket& sock, TexturePaths& texturesFromServer) {
	bool acceptingAliens = true;
	while (acceptingAliens) {
		co_await sock.asyncRead(lobbyUpdate);

		switch (lobbyUpdate) {
		case StartingGame:
			acceptingAliens = false;
			break;
		case PacifistJoining:
			texturesFromServer.push("Cosmic_Encounter/game_assets/texture_objects/pacifist.nv_texture_object");
			break;
		case VirusJoining:
			texturesFromServer.push("Cosmic_Encounter/game_assets/texture_objects/virus.nv_texture_object");
			break;
		}
	}
}

static void checkForLobbyUpdate(nv::Scene& lobby, int& alienX, const int alienY) {
	auto loadAlien = [&](std::string_view relativePath) {
		nv::TextureObject alien{
			lobby.renderer,
			nv::parseFile(nv::relativePath(relativePath)),
			lobby.texMap
		};
		alien.setPos(alienX, alienY);
		alienX += 300;
		lobby.textures[0].push_back(std::move(alien));
	};
	std::unique_lock lock{ mutex };
	if (isLobbyUpdate) {
		isLobbyUpdate = false;
		switch (lobbyUpdate) {
		case StartingGame:
			lobby.running = false;
			break;
		case PacifistJoining:
			loadAlien("Cosmic_Encounter/game_assets/texture_objects/pacifist.nv_texture_object");
			break;
		case VirusJoining:
			loadAlien("Cosmic_Encounter/game_assets/texture_objects/virus.nv_texture_object");
			break;
		}
		hasLoadedAlien = true;
		lock.unlock();
		cv.notify_one();
	}
}

std::vector<nv::TextureObject> runLobby(Socket& sock, nv::TextureObject& alien, SDL_Renderer* renderer, nv::TextureMap& texMap, nv::FontMap& fontMap) {
	nv::Scene lobby{ nv::relativePath("Cosmic_Encounter/game_assets/scenes/lobby.nv_scene"), renderer, texMap, fontMap };
	
	int alienX = 10;
	const int alienY = 500;

	alien.setPos(alienX, alienY);
	lobby.textures[0].push_back(alien);

	asio::co_spawn(sock.getExecutor(), checkForLobbyUpdates(sock), asio::use_awaitable);

	lobby.eventHandler.addPeriodicEvent([&] { checkForLobbyUpdate(lobby, alienX, alienY); });
	
	lobby();
}
