#pragma once

#include <vector>

#include <boost/geometry.hpp>

#include <SDL2/SDL_rect.h>

namespace nv {
	class CollisionShape {
	private:
		std::vector<SDL_Point> m_lines;
	public:
		bool isContained(const CollisionShape& other) noexcept;
	};
}