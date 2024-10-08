#pragma once

#include "novalis/data_util/Reflection.h"
#include "novalis/Instance.h"
#include "novalis/Sprite.h"
#include "novalis/Text.h"

#include "Game.h"

#include <boost/unordered/unordered_flat_map.hpp>

using CardMap  = boost::unordered_flat_map<Card, nv::Texture, nv::HashAggregate, nv::CompareAggregates>;
using ColorMap = boost::unordered_flat_map<Color, std::pair<nv::Texture, nv::Rect>>;

struct GameRenderData {
	std::vector<nv::Texture> pCards;
	SDL_Renderer* renderer;
	nv::TextureMap texMap;
	nv::FontMap fontMap;
	CardMap cardMap;
	ColorMap colorMap;
	Color pColor{};
	GameRenderData(SDL_Renderer* renderer) noexcept;
};