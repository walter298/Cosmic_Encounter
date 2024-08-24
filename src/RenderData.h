#pragma once

#include "novalis/Sprite.h"

#include "Game.h"

using CardMap  = std::unordered_map<Card, nv::Texture>;
using ColorMap = std::unordered_map<Color, std::pair<nv::Texture, nv::Rect>>;

struct GameRenderData {
	std::vector<nv::Texture> pCards;
	CardMap cardMap;
	ColorMap colorMap;
	Color pColor{};
	GameRenderData(SDL_Renderer* renderer, nv::TextureMap& texMap);
};