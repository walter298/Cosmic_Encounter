#pragma once

#include "novalis/Scene.h"

#include "Game.h"

class PlanetLayout : public nv::Scene {
private:

public:
	PlanetLayout(const std::string& path, nv::Instance& instance, Color color);
};

