#ifndef RECT_H
#define RECT_H

#include <iostream>
#include <memory>

#include <SDL2/SDL_render.h>
#include <SDL2/SDL_rect.h>

#include "DataUtil.h"

namespace nv {
	struct Rect : public NamedObject {
		SDL_Renderer* renderer = nullptr;
	
		static bool isInRegion(int mx, int my, int x, int y, int w, int h) noexcept;
		static bool isInRegion(const Coord& coord, int x, int y, int w, int h) noexcept;

		SDL_Rect rect{ 0, 0, 0, 0 };
		SDL_Color color{ 255, 255, 255, 255 };
		int borderThickness = 20;

		Rect() = default;
		Rect(SDL_Renderer* renderer, int x, int y, int w, int h, Uint8 r = 255, Uint8 g = 255, Uint8 b = 255, Uint8 a = 255);
		
		bool containsCoord(int mX, int mY) const noexcept;
		bool containsCoord(SDL_Point p) const noexcept;

		void move(int dx, int dy) noexcept;
		void move(SDL_Point p) noexcept;

		void scale(int dw, int dh) noexcept;
		void scale(SDL_Point p) noexcept;

		void setPos(int x, int y) noexcept;
		void setPos(SDL_Point p) noexcept;

		void setSize(int w, int h) noexcept;
		void setSize(SDL_Point p) noexcept;

		SDL_Point getSize() const noexcept;
		void render() const noexcept;
		
		void setRenderColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) noexcept;

		void save(json& json) const;

		friend void to_json(json& j, const Rect& r);
		friend void from_json(const json& j, Rect& r);
	};

	void to_json(json& j, const Rect& r);
	void from_json(const json& j, Rect& r);

	using RectPtr = std::shared_ptr<Rect>;
}

#endif