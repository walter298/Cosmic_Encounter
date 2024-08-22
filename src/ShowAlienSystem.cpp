#include "ShowAlienSystem.h"

namespace {
	constexpr int PLANET_LAYER = 1;
	constexpr int UFO_CURSOR_LAYER = 10;

	size_t colonyIdx = 0;
}

nv::Scene& getPlanetSelector(nv::Scene& showHomeSystem) {
	static nv::Scene ret{
		nv::relativePath("Cosmic_Encounter/game_assets/scenes/planet_selector_overlay.nv_scene"),
		showHomeSystem.renderer,
		showHomeSystem.texMap,
		showHomeSystem.fontMap
	};
	ret.addEvent([ufoCursor = ret.find<nv::SpriteRef>(UFO_CURSOR_LAYER, "ufo_cursor")](nv::MouseData mouse) {
		auto hoveredTexIdx = ufoCursor.get().containsCoord(mouse.x, mouse.y);
		if (hoveredTexIdx) {
			ufoCursor.get().setOpacity(255);
			ufoCursor.get().setPos(mouse.x, mouse.y);
			if (mouse.left == nv::MouseButtonState::Down) {
				colonyIdx   = *hoveredTexIdx;
				ret.running = false;
			}
		} else {
			ufoCursor.get().setOpacity(0);
		}
	});
	ret.overlay(showHomeSystem);
	return ret;
}

size_t selectPlanet(nv::Scene& planetSelector, Color color) {
	auto& ufoCursor = planetSelector.find<nv::Sprite>(UFO_CURSOR_LAYER, "ufo_cursor").get();
	ufoCursor.setTextureLayer(static_cast<int>(color));
	planetSelector();
	return colonyIdx;
}

