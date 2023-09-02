#pragma once

#include "novalis/Scene.h"

#include "RenderLayout.h"

class Destiny : public nv::Scene {
private:
	std::vector<nv::Sprite*> m_alienSprites;
public:
	Destiny(nv::Instance& instance, std::vector<nv::Sprite*> sprites);
};

