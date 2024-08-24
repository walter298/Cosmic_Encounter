#pragma once

#include "novalis/Scene.h"

#include "Game.h"

class PlanetSelector {
private:
	static constexpr int PLANET_LAYER = 1;
	static constexpr int UFO_CURSOR_LAYER = 10;
	nv::Scene m_scene;
	size_t m_colonyIdx = 0;
public:
	PlanetSelector(nv::Scene& showHomeSystem);

	size_t operator()(Color color);
};

nv::Scene& getPlanetSelector(nv::Scene& showHomeSystem);
size_t selectPlanet(nv::Scene& planetSelector, Color color);