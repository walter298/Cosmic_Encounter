#pragma once

#include "novalis/Scene.h"

#include "NetworkUtil.h"
#include "RenderData.h"

inline constexpr int ACCEPTED_COLOR = 0;
inline constexpr int DECIDED_TO_KEEP_DRAWING = 1;

void showDestiny(Socket& sock, nv::Scene& mainUi, const ColorMap& colors, Color pColor, bool takingTurn);