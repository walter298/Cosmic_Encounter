#pragma once

#include <boost/geometry.hpp>

#include "DataUtil.h"

namespace nv {
	class CollisionShape {
	private:
		std::vector<SDL_Point> m_lines;
	public:
		bool isContained(const CollisionShape& other) noexcept;
	};
}