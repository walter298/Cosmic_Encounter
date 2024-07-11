#include "ClientSide.h"

#include <algorithm>
#include <array>
#include <ranges>
#include <vector>

#include "novalis/Instance.h"

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

void play(const tcp::endpoint& endpoint) {
	asio::io_context context;
	
	nv::Instance instance{ "Cosmic Encounter" };
	nv::TextureMap texMap;

	auto cardSprites = loadCardSprites(instance.renderer, texMap);
	ranges::sort(cardSprites, [](const auto& card1, const auto& card2) {
		return card1.getName() < card2.getName();
	});


}