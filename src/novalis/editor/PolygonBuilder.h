#pragma once

#include <optional>
#include <vector>

#include "../Collision.h"
#include "SpecialPoint.h"

namespace nv {
	namespace editor {
		class PolygonBuilder {
		private:
			SDL_Renderer* m_renderer;
			bool m_placingNewPoint = false;

			Polygon m_polygon;
			SpecialPoint m_firstPoint;
			SpecialPoint m_lastPlacedPoint;
		public:
			bool building = false;

			PolygonBuilder(SDL_Renderer* m_renderer) noexcept;

			std::optional<Polygon> operator()(SDL_Point mouse) noexcept;
		};
	}
}

