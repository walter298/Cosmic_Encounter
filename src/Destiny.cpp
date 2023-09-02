#include "Destiny.h"

Destiny::Destiny(nv::Instance& instance, std::vector<nv::Sprite*> alienSprites)
	: Scene("ff", instance), m_alienSprites(std::move(alienSprites))
{
	int currX = 20;
	for (auto& obj : m_alienSprites) {
		obj->setRenPos(currX, 700);
		currX += (obj->getRen().rect.w + 20);
	}
}

