#pragma once

#include <SDL2/SDL_render.h>

#include "NetworkUtil.h"

void joinGame(SDL_Renderer* renderer, tcp::socket& sock);