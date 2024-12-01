#pragma once

#include <boost/container/static_vector.hpp>

#include "novalis/Scene.h"

#include "RenderData.h"

struct ShowAlienSystem {
	static constexpr int PLANET_LAYER = 1;
	static constexpr int SHIP_COUNT_LAYER = 4;

	class ShipCountShower {
	private:
		SDL_Renderer* m_renderer;
		TTF_Font* m_font;

		struct ShipCount {
			SDL_Color color;
			nv::Text* text = nullptr;
		};
		using ShipCounts = boost::container::static_vector<ShipCount, 6>;
		mutable boost::unordered_flat_map<SDL_Point, ShipCounts, nv::HashAggregate, nv::CompareAggregates> m_shipCounts;

		boost::unordered_flat_map<int, nv::Text> m_loadedText;
	public:
		const std::string name = "ship_count_shower";
		void push(SDL_Renderer* renderer, int shipCount, SDL_Color color, ShipCounts& shipCounts);
		void set(SDL_Renderer* renderer, const nv::Subrange<Colonies>& colonies, const std::vector<nv::Texture>& planets, const ColorMap& colorMap);
		void render(SDL_Renderer* renderer) const noexcept;
		ShipCountShower(TTF_Font* font) noexcept;
	};

	nv::Scene scene;
	nv::Sprite& planets;
	ShipCountShower shipCountShower;

	ShowAlienSystem(SDL_Renderer* renderer, nv::TextureMap& texMap, nv::FontMap& fontMap);
};

class PlanetSelector {
private:
	static constexpr int UFO_CURSOR_LAYER = 10;
	
	nv::Scene m_scene;
	nv::Sprite& m_planets;
	nv::Sprite& m_ufoCursor;
	ShowAlienSystem::ShipCountShower& m_shipCountShower;

	size_t m_colonyIdx = 0;
public:
	PlanetSelector(ShowAlienSystem& showAlienSystem);

	size_t operator()(SDL_Renderer* renderer, Color color, const nv::Subrange<Colonies>& colonies, const ColorMap& colorMap);
};