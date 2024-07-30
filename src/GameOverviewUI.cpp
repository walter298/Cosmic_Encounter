#include "GameOverviewUI.h"

#include <algorithm>

#include "novalis/Scene.h"

#include "Destiny.h"
#include "RenderData.h"

namespace ranges = std::ranges;

static constexpr size_t NEGOTIATE_CARD_IDX = 0;

using CardMap = std::unordered_map<Card, nv::TextureObject>;

static std::vector<nv::TextureObject> readCardsFromServer(Socket& sock, const CardMap& allCardObjs) {
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

void showGameOverview(Socket& sock, SDL_Renderer* renderer, nv::TextureMap& texMap,nv::FontMap& fontMap)
{
	nv::Scene mainUi{ nv::relativePath("Cosmic_Encounter/game_assets/scenes/constant_game_ui.nv_scene"), renderer, texMap, fontMap };
	nv::Scene destiny{ nv::relativePath("Cosmic_Encounter/game_assets/scenes/destiny.nv_scene"), renderer, texMap, fontMap };

	GameRenderData renderData{ renderer, texMap };

	constexpr int CARD_LAYER = 0;
	mainUi.textures[CARD_LAYER].append_range(readCardsFromServer(sock, renderData.cardMap));

	std::vector<Color> turnOrder;
	sock.read(turnOrder);
	size_t colorTurnIdx = 0;

	mainUi.overlay(destiny);
	
	showDestiny(sock, mainUi, renderData.colorMap, renderData.pColor == turnOrder[colorTurnIdx]);
	//mainUi();
}
