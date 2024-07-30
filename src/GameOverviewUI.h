#pragma once

#include "novalis/Text.h"
#include "novalis/Texture.h"

#include "Game.h"
#include "NetworkUtil.h"

void showGameOverview(Socket& sock, SDL_Renderer* renderer, nv::TextureMap& texMap, nv::FontMap& fontMap);