#include "GameOverviewUI.h"

#include <algorithm>

#include "novalis/Scene.h"

#include "Game.h"

namespace ranges = std::ranges;

static constexpr size_t NEGOTIATE_CARD_IDX = 0;

using CardObjects = std::unordered_map<Card, nv::TextureObject>;

static std::vector<nv::TextureObject> readCardsFromServer(Socket& sock, const CardObjects& allCardObjs) {
	std::vector<nv::TextureObject> ret;

	//read cards sent over from server
	std::vector<Card> cards;
	cards.reserve(10); //reserve 10 just in case
	sock.read(cards);
	
	//add cards
	ret.reserve(cards.size());
	for (const auto& card : cards) {
		ret.push_back(allCardObjs.at(card));
	}

	//set the position of the cards
	int cardX = 300;
	constexpr int cardY = 1050;
	for (auto& card : ret) {
		card.setPos(cardX, cardY);
		cardX += card.getSize().x + 20;
	}

	return ret;
}

static CardObjects loadCardObjects(SDL_Renderer* renderer) {
	CardObjects ret;

	namespace fs = std::filesystem;

	nv::TextureData texData;
	texData.ren.setSize(200, 200);

	//load negotiate
	auto negPath = nv::relativePath("Cosmic_Encounter/game_assets/images/cards/negotiate/negotiate.png");
	nv::SharedTexture tex{ IMG_LoadTexture(renderer, negPath.c_str()), SDL_DestroyTexture };
	ret.emplace(std::piecewise_construct,
		std::forward_as_tuple(Card::Negotiate), 
		std::forward_as_tuple(renderer, negPath, tex, texData)
	);
	
	auto loadImpl = [&](const fs::directory_entry& image, Card::Type type) {
		auto pathStr = image.path().string();
		int attack = std::stoi(image.path().filename().string());
		nv::SharedTexture tex{ IMG_LoadTexture(renderer, pathStr.c_str()), SDL_DestroyTexture };
		ret.emplace(std::piecewise_construct, 
			std::forward_as_tuple(Card::Attack, attack),
			std::forward_as_tuple(renderer, pathStr, tex, texData)
		);
	};

	//load sprites and reinforcements
	for (const auto& image : fs::directory_iterator{ nv::relativePath("Cosmic_Encounter/game_assets/images/cards/attack") }) {
		loadImpl(image, Card::Attack);
	}
	for (const auto& image : fs::directory_iterator{ nv::relativePath("Cosmic_Encounter/game_assets/images/cards/reinforcement") }) {
		loadImpl(image, Card::Reinforcement);
	}

	return ret;
}

void showGameOverview(Socket& sock, SDL_Renderer* renderer, std::vector<nv::TextureObject>& aliens, nv::TextureMap& texMap, nv::FontMap& fontMap) {
	nv::Scene scene{ nv::relativePath("Cosmic_Encounter/game_assets/scenes/constant_game_ui.nv_scene"), renderer, texMap, fontMap };

	//load cards in scene
	auto cardObjs = loadCardObjects(renderer);
	constexpr int CARD_LAYER = 0;

	scene.textures[CARD_LAYER].append_range(readCardsFromServer(sock, cardObjs));

	scene();
}
