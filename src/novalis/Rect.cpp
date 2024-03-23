#include "Rect.h"

nv::Rect::Rect(int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
	: rect{ x, y, w, h }, color{ r, g, b, a }
{}

bool nv::Rect::isCoordContained(const int& mX, const int& mY) const noexcept {
	return (mX > rect.x && mX < rect.x + rect.w &&
		mY > rect.y && mY < rect.y + rect.h);
}

void nv::Rect::move(int dx, int dy) noexcept {
	rect.x += dx;
	rect.y += dy;
}
void nv::Rect::setSize(int w, int h) noexcept {
	rect.w = w;
	rect.h = h;
}
void nv::Rect::scale(int dw, int dh) noexcept {
	rect.w += dw;
	rect.h += dh;
}
void nv::Rect::setPos(int x, int y) noexcept {
	rect.x = x;
	rect.y = y;
}

void nv::Rect::render(SDL_Renderer* renderer) noexcept {
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	SDL_RenderFillRect(renderer, &rect);
}

void nv::Rect::setRenderColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) noexcept {
	color.r = r, color.g = g, color.b = b, color.a = a;
}