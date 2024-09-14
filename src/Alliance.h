#pragma once

#include "novalis/Scene.h"

#include "RenderData.h"

class Alliance {
public:
	enum class Choice {
		JoiningOffense,
		JoiningDefense,
		NotJoining
	};
private:
	static constexpr int ALIEN_RECT_LAYER   = 2;
	static constexpr int BUTTON_RECT_LAYER  = 3;
	static constexpr int BUTTON_TEXT_LAYER  = 4;
	static constexpr int ALIEN_LAYER        = 5;

	nv::Scene m_scene;
	
	ColorMap& m_colorMap;

	nv::Text& m_joinOffenseText;
	nv::Text& m_joinDefenseText;

	SDL_Point m_acceptOffenseTextPos;
	nv::Rect& m_offenseButtonRect;

	SDL_Point m_acceptDefenseTextPos;
	nv::Rect& m_defenseButtonRect;

	Choice m_allianceChoice{};

	nv::Button makeButton(nv::Text& text, nv::Rect& rect, Choice choice);
public:
	Alliance(GameRenderData& gameRenderData);

	Choice operator()(Color offense, Color defense, bool invitedToOffense, bool invitedToDefense);
};

