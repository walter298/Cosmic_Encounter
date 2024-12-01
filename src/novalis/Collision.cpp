#include "Collision.h"

void nv::Polygon::move(int dx, int dy) noexcept {
	for (BGPoint& point : rep.outer()) {
		point.set<0>(point.get<0>() + dx);
		point.set<1>(point.get<1>() + dy);
	}
}

void nv::Polygon::move(SDL_Point p) noexcept {
	move(p.x, p.y);
}

bool nv::Polygon::containsCoord(int x, int y) const noexcept {
	return bg::intersects(rep, BGPoint{ x, y });
}

bool nv::Polygon::containsCoord(SDL_Point p) const noexcept {
	return containsCoord(p.x, p.y);
}

void nv::Polygon::render(SDL_Renderer* renderer) const noexcept {
	SDL_SetRenderDrawColor(renderer, 0, 255, 0, opacity);
	auto points = std::views::zip(rep.outer(), rep.outer() | std::views::drop(1));
	for (const auto& [p1, p2] : points) {
		SDL_RenderDrawLine(renderer, p1.get<0>(), p1.get<1>(), p2.get<0>(), p2.get<1>());
	}
	auto first = rep.outer().front();
	auto last = rep.outer().back();
	SDL_RenderDrawLine(renderer, first.get<0>(), first.get<1>(), last.get<0>(), last.get<1>());
}

void nv::Polygon::setOpacity(uint8_t opacityP) noexcept {
	opacity = opacityP;
}

void nv::Polygon::save(json& j) const {
	j = rep;
}

SDL_Point nv::toSDLPoint(BGPoint p) noexcept {
	return { p.get<0>(), p.get<1>() };
}
