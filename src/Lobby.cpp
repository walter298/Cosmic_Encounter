#include "Lobby.h"

#include <algorithm>
#include <mutex>

#include "novalis/data_util/BasicJsonSerialization.h"
#include "novalis/data_util/File.h"
#include "novalis/Scene.h"

#include "Game.h"

namespace {
	class AlienPositionSetter {
	private:
		int m_alienX = 0;
		static constexpr int ALIEN_Y = 500;
	public:
		void setPos(nv::Texture& alien) {
			alien.setPos(m_alienX, ALIEN_Y);
			m_alienX += alien.getSize().x + 20;
		}
	};

	std::string getAlienTexPath(Alien alien) {
		switch (alien) {
		case Pacifist:
			return "Cosmic_Encounter/game_assets/texture_objects/pacifist.nv_texture_object";
			break;
		case Virus:
			return "Cosmic_Encounter/game_assets/texture_objects/virus.nv_texture_object";
			break;
		}
		//invalid texture path
		std::abort();
	}

	nv::Rect getColorRect(SDL_Renderer* renderer, Color color) {
		constexpr int CARD_WIDTH = 200;
		constexpr int CARD_HEIGHT = 200;
		switch (color) {
		case Blue:
			return nv::Rect{ renderer, 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 0, 255 };
			break;
		case Black:
			return nv::Rect{ renderer, 0, 0, CARD_WIDTH, CARD_HEIGHT };
			break;
		case Green:
			return nv::Rect{ renderer, 0, 0, CARD_WIDTH, CARD_HEIGHT, 0, 178, 0 };
			break;
		case Purple:
			return nv::Rect{ renderer, 0, 0, CARD_WIDTH, CARD_HEIGHT, 138, 43, 226 };
			break;
		case Red:
			return nv::Rect{ renderer, 0, 0, CARD_WIDTH, CARD_HEIGHT, 255, 0, 0 };
			break;
		}
		assert(false); //invalid color
		std::unreachable();
	}

	asio::awaitable<void> readServerUpdates(Socket& sock, size_t pCount, 
		std::vector<PlayerRenderData>& pRenderDataV, std::mutex& mutex, 
		std::atomic_bool& gameStarting) 
	{
		PlayerRenderData pRenderData;
		size_t alienJoinCount = 0;
		while (alienJoinCount < pCount) {
			co_await sock.asyncRead(SocketHeader::NEWLY_JOINED_PLAYER, pRenderData);
			alienJoinCount++;

			std::scoped_lock lock{ mutex };
			pRenderDataV.push_back(pRenderData);
		} 
		
		co_await sock.asyncRead(SocketHeader::STARTING_GAME);
		gameStarting.store(true);
	}

	void loadColorData(AlienPositionSetter& alienPosSetter, nv::Scene& lobby, 
		PlayerRenderData pRenderInfo, ColorMap& colorMap) 
	{
		nv::Texture alien{
			lobby.renderer,
			nv::parseJson(nv::relativePath(getAlienTexPath(pRenderInfo.alien))),
			lobby.texMap
		};
		alienPosSetter.setPos(alien);
		colorMap[pRenderInfo.color] = { alien, getColorRect(lobby.renderer, pRenderInfo.color) };
		lobby.addObject(std::move(alien), 1);
	}

	bool checkForLobbyUpdate(nv::Scene& lobby, AlienPositionSetter& alienPosSetter,
		std::vector<PlayerRenderData>& pRenderInfoV, size_t& alienJoinCount, size_t pCount, 
		std::mutex& mutex, ColorMap& colorMap) 
	{
		std::scoped_lock lock{ mutex };
		while (!pRenderInfoV.empty()) {
			auto pRenderData = std::move(pRenderInfoV.back());
			loadColorData(alienPosSetter, lobby, pRenderInfoV.back(), colorMap);
			pRenderInfoV.pop_back();
			alienJoinCount++;
		}
		return alienJoinCount == pCount;
	}
}

GameRenderData runLobby(Socket& sock, SDL_Renderer* renderer, nv::TextureMap& texMap, nv::FontMap& fontMap) {
	GameRenderData ret{ renderer, texMap };

	nv::Scene lobby{ nv::relativePath("Cosmic_Encounter/game_assets/scenes/lobby.nv_scene"), renderer, texMap, fontMap };

	size_t pCount = 0;
	std::vector<PlayerRenderData> pRenderInfoV;
	pRenderInfoV.reserve(5);
	sock.read(SocketHeader::ALREADY_ARRIVED_PLAYERS, pCount, pRenderInfoV);

	assert(!pRenderInfoV.empty());
	assert(pCount != 0);

	AlienPositionSetter alienPosSetter;
	
	//load already joined aliens
	for (const auto& pData : pRenderInfoV) {
		loadColorData(alienPosSetter, lobby, pData, ret.colorMap);
	}
	ret.pColor = pRenderInfoV.back().color; //last sent player is THIS player

	pCount -= pRenderInfoV.size(); //subtract from pCount the # of players who have already joined
	pRenderInfoV.clear(); 

	std::mutex mutex; //protects pRenderInfoV
	std::atomic_bool gameStarting = false; 

	//asynchrously read alien texture paths off the network
	asio::co_spawn(sock.getExecutor(), readServerUpdates(sock, pCount, pRenderInfoV, mutex, gameStarting), asio::detached);

	//periodically check server updates
	lobby.addEvent([&, alienJoinCount = size_t{}]() mutable {
		if (gameStarting.load()) {
			lobby.running = false;
		} else {
			checkForLobbyUpdate(lobby, alienPosSetter, pRenderInfoV, alienJoinCount, pCount, mutex, ret.colorMap);
		}
	});

	lobby();
	
	return ret;
}
