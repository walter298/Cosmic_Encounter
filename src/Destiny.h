#pragma once

#include "novalis/Scene.h"

#include "Game.h"
#include "NetworkUtil.h"

using ColorMap = std::unordered_map<Color, nv::Rect>;

void showDestiny(Socket& sock, nv::Scene& mainUi, const ColorMap& colors, bool takingTurn);