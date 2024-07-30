#pragma once

#include "novalis/Sprite.h"

#include "Game.h"

//todo: get rid of duplicate declarations
using CardMap  = std::unordered_map<Card, nv::TextureObject>;
using ColorMap = std::unordered_map<Color, std::pair<nv::TextureObject, nv::Rect>>;

struct GameRenderData {
	nv::Sprite planets;
	std::vector<nv::TextureObject> pCards;
	CardMap cardMap;
	ColorMap colorMap;
	Color pColor{};
	GameRenderData(SDL_Renderer* renderer, nv::TextureMap& texMap);
};