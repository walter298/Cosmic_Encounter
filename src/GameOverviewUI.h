#pragma once

#include "novalis/Text.h"
#include "novalis/Texture.h"

#include "NetworkUtil.h"

void showGameOverview(Socket& sock, SDL_Renderer* renderer, std::vector<nv::TextureObject>& aliens, nv::TextureMap& texMap, nv::FontMap& fontMap);