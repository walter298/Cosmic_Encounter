#include "ShowAlienSystem.h"

ShowAlienSystem::ShipCountShower::ShipCountShower(TTF_Font* shipCountFont) noexcept
	: m_font{ shipCountFont }
{
}

void ShowAlienSystem::ShipCountShower::push(SDL_Renderer* renderer, int shipCount, SDL_Color color, ShipCounts& shipCounts) {
	static std::string lastParsedInt;

	auto textIt = m_loadedText.find(shipCount);
	if (textIt == m_loadedText.end()) {
		lastParsedInt = std::to_string(shipCount);
		auto it = m_loadedText.emplace(std::piecewise_construct, std::forward_as_tuple(shipCount), std::forward_as_tuple(renderer, lastParsedInt, 20, m_font));
		auto& text = it.first->second;
		shipCounts.emplace_back(color, &text);
	} else {
		shipCounts.emplace_back(color, &textIt->second);
	}
}

void ShowAlienSystem::ShipCountShower::set(SDL_Renderer* renderer, const nv::Subrange<Colonies>& colonies, const std::vector<nv::Texture>& planets, const ColorMap& colorMap) {
	m_shipCounts.clear();
	for (const auto [colony, planet] : std::views::zip(colonies, planets)) {
		auto& renderedShipCounts = m_shipCounts[planet.getPos()];
		for (const auto& [color, shipCount] : colony.ships) {
			push(renderer, shipCount, colorMap.at(color).second.color, renderedShipCounts);
		}
	}
}

void ShowAlienSystem::ShipCountShower::render(SDL_Renderer* renderer) const noexcept {
	assert(!m_shipCounts.empty());
	for (auto& [pos, shipCounts] : m_shipCounts) {
		SDL_Point currPos = pos;
		for (auto& [color, text] : shipCounts) {
			text->ren.setPos(currPos);
			text->ren.color = color;
			text->render(renderer);
			currPos.y += text->ren.rect.h;
		}
	}
}

ShowAlienSystem::ShowAlienSystem(SDL_Renderer* renderer, nv::TextureMap& texMap, nv::FontMap& fontMap)
	: scene{
		nv::relativePath("Cosmic_Encounter/game_assets/scenes/home_system.nv_scene"),
		renderer,
		texMap,
		fontMap
	},
	planets{ scene.find<nv::Sprite>(PLANET_LAYER, "planets").get() },
	shipCountShower{ fontMap.begin()->second.get() }
{
	scene.addCustomObject(std::ref(shipCountShower), SHIP_COUNT_LAYER);
}

PlanetSelector::PlanetSelector(ShowAlienSystem& showAlienSystem)
	: m_scene{
		nv::relativePath("Cosmic_Encounter/game_assets/scenes/planet_selector_overlay.nv_scene"),
		showAlienSystem.scene.renderer,
		showAlienSystem.scene.texMap,
		showAlienSystem.scene.fontMap
	},
	m_planets{ showAlienSystem.planets },
	m_ufoCursor{ m_scene.find<nv::Sprite>(UFO_CURSOR_LAYER, "ufo_cursor").get() },
	m_shipCountShower{ showAlienSystem.shipCountShower }
{
	m_scene.overlay(showAlienSystem.scene);
	m_scene.addEvent([this](nv::MouseData mouse) mutable {
		auto hoveredTexIdx = m_planets.containsCoord(mouse.x, mouse.y);
		if (hoveredTexIdx) {
			m_ufoCursor.setOpacity(255);
			m_ufoCursor.setPos(mouse.x - 30, mouse.y - 30);
			/*if (mouse.left == nv::MouseButtonState::Down) {
				m_colonyIdx = *hoveredTexIdx;
				m_scene.running = false;
			}*/
		} else {
			m_ufoCursor.setOpacity(0);
		}
	});
}

size_t PlanetSelector::operator()(SDL_Renderer* renderer, Color color, const nv::Subrange<Colonies>& colonies, const ColorMap& colorMap) {
	m_planets.setTextureLayer(static_cast<int>(color));
	m_ufoCursor.setTextureLayer(static_cast<int>(color));
	m_shipCountShower.set(renderer, colonies, m_planets.getTextures(), colorMap);

	m_scene();
	return m_colonyIdx;
}