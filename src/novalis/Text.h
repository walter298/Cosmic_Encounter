#pragma once

#include <functional>
#include <memory>
#include <string>
#include <string_view>

#include <boost/unordered/unordered_flat_map.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "data_util/BasicConcepts.h"
#include "Event.h"
#include "Texture.h"

namespace nv {
	using FontRAII = std::unique_ptr<TTF_Font, void(*)(TTF_Font*)>;
	using FontMap  = boost::unordered_flat_map<std::string, FontRAII>;

	FontRAII loadFont(std::string_view fontPath, int fontSize);

	class Text : public ObjectBase<Text> {
	private:
		SDL_Renderer* m_renderer;
		TTF_Font* m_font;
		SharedTexture m_tex = nullptr; //points to the pointer in the variant
		std::string m_str;
		std::string m_fontPath;
		int m_fontSize = 0;
		void changeText(std::string_view newText) noexcept;
	public:
		Rect ren;
		SDL_Color color{ 0, 0, 0, 255 };

		Text(SDL_Renderer* renderer, std::string_view str, std::string_view fontPath, int fontSize, TTF_Font* font);
		Text(SDL_Renderer* renderer, std::string_view str, int fontSize, TTF_Font* font);

		Text(SDL_Renderer* renderer, const json& json, FontMap& fontMap);

		void operator=(std::string_view str) noexcept;

		std::string_view value() const noexcept;

		void move(int dx, int dy) noexcept;
		void move(SDL_Point p) noexcept;
		void setPos(int x, int y) noexcept;
		void setPos(SDL_Point p) noexcept;
		SDL_Point getPos() const noexcept;
		void scale(int dw, int dh) noexcept;
		void scale(SDL_Point p) noexcept;
		bool containsCoord(int x, int y) const noexcept;
		bool containsCoord(SDL_Point p) const noexcept;
		void setOpacity(uint8_t a) noexcept;
		void render() const noexcept;

		void save(json& json) const;
	};

	using TextRef = std::reference_wrapper<Text>;

	class TextInput {
	private:
		Rect m_rect;
		std::reference_wrapper<Text> m_text;
		bool m_mouseClickedAndInRegion = false;
		std::string m_buff;

		chrono::system_clock::time_point m_lastTimePopped = chrono::system_clock::now();
		bool tooSoonToPop() const noexcept;
	public:
		TextInput(const Rect& rect, Text& text);

		const nv::Rect& getRect() const noexcept;

		void append(std::string_view inputText);
		void pop();
	};
}