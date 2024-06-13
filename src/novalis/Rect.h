#ifndef RECT_H
#define RECT_H

#include <iostream>
#include <memory>

#include <SDL2/SDL_render.h>
#include <SDL2/SDL_rect.h>

#include "DataUtil.h"

namespace nv {
	struct Rect {
		static bool isInRegion(int mx, int my, int x, int y, int w, int h) noexcept;
		static bool isInRegion(const Coord& coord, int x, int y, int w, int h) noexcept;

		SDL_Rect rect{ 0, 0, 0, 0 };
		SDL_Color color{ 255, 255, 255, 255 };
		int borderThickness = 20;

		Rect() = default;
		Rect(int x, int y, int w, int h, Uint8 r = 255, Uint8 g = 255, Uint8 b = 255, Uint8 a = 255);

		bool containsCoord(int mX, int mY) const noexcept;

		enum class Border {
			Right,
			Left,
			Top,
			Bottom,
			TopRight,
			TopLeft,
			BottomRight,
			BottomLeft,
			None
		};
		Border hoveredBorder(int mx, int my, int padding = 0) const noexcept;
		Border hoveredBorder(const Coord& mousePos, int padding = 0) const noexcept;

		void move(int dx, int dy) noexcept;
		void move(const Coord& c) noexcept;

		void scale(int dw, int dh) noexcept;
		void scale(const Coord& c) noexcept;

		void setPos(int x, int y) noexcept;
		void setPos(const Coord& c) noexcept;

		void setSize(int w, int h) noexcept;
		void setSize(const Coord& c) noexcept;

		void render(SDL_Renderer* renderer) const noexcept;
		void renderBorders(SDL_Renderer* renderer) const noexcept;

		void setRenderColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) noexcept;
	};

	void to_json(json& j, const Rect& r);
	void from_json(const json& j, Rect& r);

	using RectPtr = std::shared_ptr<Rect>;
}

#endif