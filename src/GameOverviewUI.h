#pragma once

#include "novalis/Text.h"
#include "novalis/Texture.h"

#include "NetworkUtil.h"
#include "RenderData.h"

void showGameOverview(Socket& sock, SDL_Renderer* renderer, nv::TextureMap& texMap, nv::FontMap& fontMap, GameRenderData& gameRenderData);