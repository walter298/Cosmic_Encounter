#pragma once

#include "DataUtil.h"

namespace nv {
	class CollisionShape {
	private:
		std::vector<Coord> m_lines;
	public:
		bool isContained(const CollisionShape& other) noexcept;
	};
}