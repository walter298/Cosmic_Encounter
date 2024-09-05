#pragma once

#include "novalis/Text.h"
#include "novalis/Texture.h"

#include "NetworkUtil.h"
#include "RenderData.h"

//player color
//player alien
//other aliens and colors

struct PlayerRenderData {
	Alien alien{};
	Color color{};
};

GameRenderData runLobby(Socket& sock, SDL_Renderer* renderer, nv::TextureMap& texMap, nv::FontMap& fontMap);