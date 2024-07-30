#pragma once

#include "novalis/Scene.h"

#include "NetworkUtil.h"
#include "RenderData.h"

void showDestiny(Socket& sock, nv::Scene& mainUi, const ColorMap& colors, bool takingTurn);