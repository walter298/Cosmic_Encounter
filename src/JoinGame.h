#pragma once

#include <SDL2/SDL_render.h>

#include "Novalis/Scene.h"

#include "NetworkUtil.h"

void joinGame(SDL_Renderer* renderer, Socket& sock, nv::TextureMap& texMap, nv::FontMap& fontMap);