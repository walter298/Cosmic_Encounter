#pragma once

#include "novalis/RenderTypes.h"



struct Alien {
	nv::SpritePtr sprite;
	std::function<void(Game&)> power;
	GamePhase powerPhase;
};
