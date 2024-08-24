#include "ShowAlienSystem.h"

PlanetSelector::PlanetSelector(nv::Scene& showHomeSystem) 
	: m_scene{ 
		nv::relativePath("Cosmic_Encounter/game_assets/scenes/planet_selector_overlay.nv_scene"),
		showHomeSystem.renderer,
		showHomeSystem.texMap,
		showHomeSystem.fontMap
	} 
{
	m_scene.addEvent([this, ufoCursor = m_scene.find<nv::SpriteRef>(UFO_CURSOR_LAYER, "ufo_cursor")](nv::MouseData mouse) mutable {
		auto hoveredTexIdx = ufoCursor.get().containsCoord(mouse.x, mouse.y);
		if (hoveredTexIdx) {
			ufoCursor.get().setOpacity(255);
			ufoCursor.get().setPos(mouse.x, mouse.y);
			if (mouse.left == nv::MouseButtonState::Down) {
				m_colonyIdx = *hoveredTexIdx;
				m_scene.running = false;
			}
		} else {
			ufoCursor.get().setOpacity(0);
		}
	});
	m_scene.overlay(showHomeSystem);
}

size_t PlanetSelector::operator()(Color color) {
	auto& ufoCursor = m_scene.find<nv::Sprite>(UFO_CURSOR_LAYER, "ufo_cursor").get();
	ufoCursor.setTextureLayer(static_cast<int>(color));
	m_scene();
	return m_colonyIdx;
}
