#include "GameOverviewUI.h"

#include <algorithm>

#include <magic_enum.hpp>

#include "novalis/Scene.h"

#include "Destiny.h"

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

static CardMap loadCardObjects(SDL_Renderer* renderer) {
	CardMap ret;

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
			std::forward_as_tuple(type, attack),
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

static ColorMap makeColorRects(SDL_Renderer* renderer, const std::vector<Color>& colors, SDL_Point size) {
	ColorMap ret;
	ret.reserve(colors.size());

	for (const auto& color : colors) {
		std::array<Uint8, 3> rgb = { 0, 0, 0 };
		switch (color) {
		case Blue:
			rgb = { 0, 0, 255 };
			break;
		case Green:
			rgb = { 0, 255, 0 };
			break;
		case Purple:
			rgb = { 138, 43, 226 };
			break;
		case Red:
			rgb = { 255, 0, 0 };
			break;
		}
		nv::Rect r{ 0, 0, size.x, size.y, rgb[0], rgb[1], rgb[2] };
		r.renderer = renderer;
		ret[color] = std::move(r);
	}

	return ret;
}

void showGameOverview(Socket& sock, SDL_Renderer* renderer, std::vector<nv::TextureObject>& aliens, nv::TextureMap& texMap,
	nv::FontMap& fontMap, const std::vector<Color>& turnOrder, Color pColor) 
{
	nv::Scene mainUi{ nv::relativePath("Cosmic_Encounter/game_assets/scenes/constant_game_ui.nv_scene"), renderer, texMap, fontMap };
	nv::Scene destiny{ nv::relativePath("Cosmic_Encounter/game_assets/scenes/destiny.nv_scene"), renderer, texMap, fontMap };

	//load cards in scene
	auto cardMap = loadCardObjects(renderer);
	constexpr int CARD_LAYER = 0;

	mainUi.textures[CARD_LAYER].append_range(readCardsFromServer(sock, cardMap));

	auto colorRects = makeColorRects(renderer, turnOrder, cardMap.begin()->second.getSize());

	size_t colorTurnIdx = 0;

	mainUi.overlay(destiny);
	showDestiny(sock, mainUi, colorRects, turnOrder[colorTurnIdx] == pColor);
	//mainUi();
}
