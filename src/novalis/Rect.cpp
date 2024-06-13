#include "Rect.h"

#include <SDL2/SDL2_gfxPrimitives.h>

bool nv::Rect::isInRegion(int mx, int my, int x, int y, int w, int h) noexcept {
	return mx > x && mx < x + w &&
		my > y && my < y + h;
}

bool nv::Rect::isInRegion(const Coord& coord, int x, int y, int w, int h) noexcept {
	return isInRegion(coord.x, coord.y, x, y, w, h);
}

nv::Rect::Rect(int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
	: rect{ x, y, w, h }, color{ r, g, b, a }
{}

bool nv::Rect::containsCoord(int mx, int my) const noexcept {
	return isInRegion(mx, my, rect.x, rect.y, rect.w, rect.h);
}

nv::Rect::Border nv::Rect::hoveredBorder(int mx, int my, int padding) const noexcept {
	bool right = isInRegion(mx, my, (rect.x + rect.w) - borderThickness - padding, rect.y, borderThickness + padding, rect.h);
	bool bottom = isInRegion(mx, my, rect.x, rect.y - padding, rect.w, borderThickness + padding);
	bool top = isInRegion(mx, my, rect.x, (rect.y + rect.h) - borderThickness - padding, rect.w, borderThickness + padding);
	
	//check corners
	int cornerRectLen = rect.w / 5;
	/*if (Rect::isInRegion(mx, my, (rect.x + rect.w) - cornerRectLen, rect.y, cornerRectLen, cornerRectLen)) {
		return Border::BottomRight;
	} else if (Rect::isInRegion(mx, my, rect.x, rect.y, cornerRectLen, cornerRectLen)) {
		return Border::BottomLeft;
	} else if (Rect::isInRegion(mx, my, (rect.x + rect.w) - cornerRectLen, (rect.y + rect.h) - cornerRectLen, cornerRectLen, cornerRectLen)) {
		return Border::TopRight;
	} else if (Rect::isInRegion(mx, my, rect.x, (rect.y + rect.h) - cornerRectLen, cornerRectLen, cornerRectLen)) {
		return Border::TopLeft;
	} */
	//check sides with padding
	if (isInRegion(mx, my, (rect.x + rect.w) - borderThickness - padding, rect.y, borderThickness + padding, rect.h)) {
		return Border::Right;
	} else if (isInRegion(mx, my, rect.x - padding, rect.y, borderThickness + padding, rect.h)) {
		return Border::Left;
	} else if (isInRegion(mx, my, rect.x, (rect.y + rect.h) - borderThickness - padding, rect.w, borderThickness + padding)) {
		return Border::Top;
	} else if (isInRegion(mx, my, rect.x, rect.y - padding, rect.w, borderThickness + padding)) {
		return Border::Bottom;
	} else {
		return Border::None;
	}
}

nv::Rect::Border nv::Rect::hoveredBorder(const Coord& mousePos, int padding) const noexcept {
	return hoveredBorder(mousePos.x, mousePos.y, padding);
}

void nv::Rect::move(int dx, int dy) noexcept {
	rect.x += dx;
	rect.y += dy;
}
void nv::Rect::move(const Coord& c) noexcept
{
	rect.x += c.x;
	rect.y += c.y;
}
void nv::Rect::scale(int dw, int dh) noexcept {
	rect.w += dw;
	rect.h += dh;
}
void nv::Rect::scale(const Coord& c) noexcept
{
	rect.w += c.x;
	rect.h += c.y;
}
void nv::Rect::setPos(int x, int y) noexcept {
	rect.x = x;
	rect.y = y;
}
void nv::Rect::setPos(const Coord& c) noexcept
{
	rect.x = c.x;
	rect.y = c.y;
}
void nv::Rect::setSize(int w, int h) noexcept {
	rect.w = w;
	rect.h = h;
}
void nv::Rect::setSize(const Coord& c) noexcept
{
	rect.w = c.x;
	rect.h = c.y;
}

void nv::Rect::render(SDL_Renderer* renderer) const noexcept {
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	SDL_RenderFillRect(renderer, &rect);
}

#include <print>

void nv::Rect::renderBorders(SDL_Renderer* renderer) const noexcept {
	static_assert(sizeof(int) == 4, "thickLineRGBA() expects 16 bit ints");

	/*thickLineRGBA(renderer, 0, 0, 50, 0, 255, 255, 0, 20, 255);
	thickLineRGBA(renderer, 0, 0, 0, 50, 255, 255, 0, 20, 255);*/

	//top line
	thickLineRGBA(renderer, rect.x, rect.y, rect.x + rect.w, rect.y, borderThickness,
		color.r, color.g, color.b, color.a
	);
	//bottom line
	thickLineRGBA(renderer, rect.x, rect.y + rect.h, rect.x + rect.w, rect.y + rect.h, borderThickness,
		color.r, color.g, color.b, color.a
	);
	//left line
	thickLineRGBA(renderer, rect.x, rect.y, rect.x, rect.y + rect.h, borderThickness,
		color.r, color.g, color.b, color.a
	);
	//right line
	thickLineRGBA(renderer, (rect.x + rect.w), rect.y, (rect.x + rect.w), rect.y + rect.h, borderThickness,
		color.r, color.g, color.b, color.a
	);
}

void nv::Rect::setRenderColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) noexcept {
	color.r = r, color.g = g, color.b = b, color.a = a;
}

void nv::to_json(json& j, const Rect& r) {
	j["sdl_rect"] = r.rect;
	j["color"] = r.color;
}

void nv::from_json(const json& j, Rect& r) {
	r.rect = j.at("sdl_rect").get<SDL_Rect>();
	r.color = j.at("color").get<SDL_Color>();
}