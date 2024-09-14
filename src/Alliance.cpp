#include "Alliance.h"

#include "novalis/data_util/File.h"

nv::Button Alliance::makeButton(nv::Text& text, nv::Rect& rect, Choice choice) {
	return nv::Button{
		text,
		rect,
		[choice, this] {
			m_allianceChoice = choice;
			m_scene.running = false;
		},
		[&] { rect.setRenderColor(34, 139, 34, 255); },
		[&] { rect.setRenderColor(255, 255, 255, 255); }
	};
}

Alliance::Alliance(GameRenderData& gameRenderData)
	: m_scene{
		nv::relativePath("Cosmic_Encounter/game_assets/scenes/alliance.nv_scene"),
		gameRenderData.renderer,
		gameRenderData.texMap,
		gameRenderData.fontMap
	},
	m_colorMap{ gameRenderData.colorMap },
	m_joinOffenseText{ m_scene.find<nv::Text>(BUTTON_TEXT_LAYER, "offense_button_text").get() },
	m_joinDefenseText{ m_scene.find<nv::Text>(BUTTON_TEXT_LAYER, "defense_button_text").get() },
	m_acceptOffenseTextPos{ m_scene.getSpecialPoint("offense_text_position") },
	m_offenseButtonRect{ m_scene.find<nv::Rect>(BUTTON_RECT_LAYER, "offense_rect").get() },
	m_acceptDefenseTextPos{ m_scene.getSpecialPoint("defense_text_position") },
	m_defenseButtonRect{ m_scene.find<nv::Rect>(BUTTON_RECT_LAYER, "offense_rect").get() }
{
	auto& doNotParticipateText = m_scene.find<nv::Text>(BUTTON_TEXT_LAYER, "do_not_participate_text").get();
	auto& doNotParticipateRect = m_scene.find<nv::Rect>(BUTTON_RECT_LAYER, "do_not_participate_rect").get();
	m_scene.addEvent(makeButton(doNotParticipateText, doNotParticipateRect, Choice::NotJoining));
}

Alliance::Choice Alliance::operator()(Color offense, Color defense, bool invitedToOffense, bool invitedToDefense) {
	auto& offenseAlien = m_colorMap.at(offense).first;
	auto& defenseAlien = m_colorMap.at(defense).first;

	auto initAlien = [this](nv::Texture alien, std::string_view alienRectName, std::string_view positionName) {
		//fit and size alien in the right position
		const auto& rect = m_scene.find<nv::Rect>(ALIEN_RECT_LAYER, alienRectName).get();
		alien.setPos(rect.getPos());
		alien.setSize(rect.getSize());
		m_scene.addObject(std::move(alien), ALIEN_LAYER);
	};

	initAlien(offenseAlien, "offense_alien_rect", "offense_button_position");
	initAlien(defenseAlien, "defense_alien_rect", "defense_button_position");

	//important that we copy buttonText and buttonRect in case they need to be duplicated
	auto makeTempButton = [this](SDL_Point textPos, nv::Rect rect, Choice choice) {
		auto textCopy = m_joinOffenseText;

		textCopy.setPos(textPos);
		
		auto& addedText = m_scene.addObject(std::move(textCopy), BUTTON_TEXT_LAYER).get();
		auto& addedRect = m_scene.addObject(std::move(rect), BUTTON_RECT_LAYER).get();
		return makeButton(addedText, addedRect, choice);
	};

	auto run = [this](auto&&... buttons) {
		std::tuple buttonIDs{ m_scene.addEvent(std::move(buttons))... };
		m_scene();
		nv::forEachDataMember([this](const auto& ID) {
			m_scene.removeEvent(ID);
			return nv::STAY_IN_LOOP;
		}, buttonIDs);
	};

	if (invitedToOffense && invitedToDefense) {
		run(makeTempButton(m_acceptOffenseTextPos, m_offenseButtonRect, Choice::JoiningOffense), makeTempButton(m_acceptDefenseTextPos, m_defenseButtonRect, Choice::JoiningDefense));
	} else if (invitedToOffense) {
		run(makeTempButton(m_acceptOffenseTextPos, m_offenseButtonRect, Choice::JoiningOffense));
	} else if (invitedToDefense) {
		run(makeTempButton(m_acceptDefenseTextPos, m_defenseButtonRect, Choice::JoiningDefense));
	}

	return m_allianceChoice;
}