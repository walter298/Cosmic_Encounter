#ifndef RECT_H
#define RECT_H

#include <iostream>
#include <memory>

#include <SDL2/SDL_render.h>
#include <SDL2/SDL_rect.h>

namespace nv {
	struct Rect {
		SDL_Rect rect{ 0, 0, 0, 0 };
		SDL_Color color{ 0, 0, 0, 0 };

		Rect() = default;
		Rect(int x, int y, int w, int h, Uint8 r = 0, Uint8 g = 0, Uint8 b = 0, Uint8 a = 0);

		bool isCoordContained(const int& mX, const int& mY) const noexcept;

		void move(int dx, int dy) noexcept;
		void setSize(int w, int h) noexcept;
		void scale(int dw, int dh) noexcept;
		void setPos(int x, int y) noexcept;

		void render(SDL_Renderer* renderer) noexcept;

		void setRenderColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) noexcept;
	};

	using RectPtr = std::shared_ptr<Rect>;
}

#endif