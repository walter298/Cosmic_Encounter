#pragma once

#include "novalis/Scene.h"

#include "NetworkUtil.h"
#include "RenderData.h"

struct TurnTakerDestinyMessage {
	Color drawnColor{};
	bool allowedToKeepDrawing = false;
	bool mustKeepDrawing = false;
};

struct NonTurnTakerDestinyMessage {
	Color drawnColor{};
	bool finalColor = 0;
};

enum DestinyResponseFromTurnTaker {
	AcceptedColor,
	DecidedToKeepDrawing
};

inline constexpr int ENDING_DESTINY_PHASE    = 2;

void showDestiny(Socket& sock, SDL_Renderer* renderer, nv::TextureMap& texMap, nv::FontMap& fontMap, const ColorMap& colorMap, bool takingTurn);