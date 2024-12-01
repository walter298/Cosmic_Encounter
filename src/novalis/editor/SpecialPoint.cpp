#include "SpecialPoint.h"

#include <cmath>

#include <SDL2/SDL2_gfxPrimitives.h>

void nv::editor::SpecialPoint::render(SDL_Renderer* renderer) const noexcept {
	filledCircleColor(renderer, static_cast<Sint16>(point.x), static_cast<Sint16>(point.y),
					  RADIUS, (255u << 24) | (255u << 16));
}

bool nv::editor::SpecialPoint::containsCoord(int x, int y) const noexcept {
	auto mx = static_cast<double>(x);
	auto my = static_cast<double>(y);

	//center points
	auto cx = static_cast<double>(point.x);
	auto cy = static_cast<double>(point.y);

	auto distFromCenter = sqrt(std::pow(mx - cx, 2.0) + std::pow(my - cy, 2.0));
	return distFromCenter < static_cast<double>(RADIUS);
}

bool nv::editor::SpecialPoint::containsCoord(SDL_Point p) const noexcept {
	return containsCoord(p.x, p.y);
}