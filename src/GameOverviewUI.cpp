#include "GameOverviewUI.h"

#include <algorithm>

#include "novalis/Scene.h"

#include "Destiny.h"
#include "RenderData.h"

#include <magic_enum.hpp>

namespace ranges = std::ranges;

static constexpr size_t NEGOTIATE_CARD_IDX = 0;

namespace {
	void showCards(nv::Scene& scene, const std::vector<Card>& cards, CardMap& cardMap) {
		constexpr int CARD_LAYER = 1;

		auto& cardLayer = scene.textures[CARD_LAYER];
		cardLayer.clear();
		if (cardLayer.capacity() < cards.size()) {
			cardLayer.reserve(cards.size());
		}

		int cardX = 300;
		constexpr int cardY = 1050;

		for (const auto& card : cards) {
			std::println("Showing Card: {} {}", magic_enum::enum_name(card.type), card.value);
			auto cardObj = cardMap.at(card); //must be copied, otherwise the same object gets pushed into textures
			cardObj.setPos(cardX, cardY);
			cardX += cardObj.getSize().x + 20;
			cardLayer.push_back(std::move(cardObj));
		}
	}
}

void showGameOverview(Socket& sock, SDL_Renderer* renderer, nv::TextureMap& texMap,nv::FontMap& fontMap, GameRenderData& gameRenderData) {
	nv::Scene mainUi{ nv::relativePath("Cosmic_Encounter/game_assets/scenes/constant_game_ui.nv_scene"), renderer, texMap, fontMap };
	nv::Scene destiny{ nv::relativePath("Cosmic_Encounter/game_assets/scenes/destiny.nv_scene"), renderer, texMap, fontMap };

	std::vector<Card> cards;
	std::vector<Color> turnOrder;
	sock.read(cards, turnOrder);

	showCards(mainUi, cards, gameRenderData.cardMap);

	size_t colorTurnIdx = 0;

	mainUi.overlay(destiny);
	
	showDestiny(sock, mainUi, gameRenderData.colorMap, gameRenderData.pColor == turnOrder[colorTurnIdx]);
	//mainUi();
}
