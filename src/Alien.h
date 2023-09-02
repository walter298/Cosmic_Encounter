#pragma once

#include "novalis/Instance.h"

#include "GameState.h"

struct Alien {
	nv::SpritePtr sprite;
	void(*power)(GameState&);
};

