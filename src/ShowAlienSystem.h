#pragma once

#include "novalis/Scene.h"

#include "Game.h"

nv::Scene& getPlanetSelector(nv::Scene& showHomeSystem);
size_t selectPlanet(nv::Scene& planetSelector, Color color);