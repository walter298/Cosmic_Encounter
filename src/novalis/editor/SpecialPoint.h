#pragma once

#include <string>

#include <SDL2/SDL_render.h>

namespace nv {
	namespace editor {
		struct SpecialPoint {
			static constexpr int RADIUS = 12;

			std::string name;
			SDL_Point point{};

			void render(SDL_Renderer* renderer) const noexcept;
			bool containsCoord(int x, int y) const noexcept;
			bool containsCoord(SDL_Point p) const noexcept;
		};
	}
}

