#pragma once

#include <memory>
#include <string>
#include <string_view>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "Event.h"
#include "Texture.h"

namespace nv {
	using FontRAII = std::unique_ptr<TTF_Font, void(*)(TTF_Font*)>;
	using FontMap  = std::unordered_map<std::string, FontRAII>;

	FontRAII loadFont(std::string_view fontPath, int fontSize);

	class Text : public NamedObject {
	private:
		SDL_Renderer* m_renderer;
		TTF_Font* m_font;
		TextureRAII m_tex{ nullptr, SDL_DestroyTexture };
		std::string m_str;
		std::string m_fontPath;
		int m_fontSize = 0;
		Rect m_ren;
		void changeText(std::string_view newText) noexcept;
	public:
		SDL_Color color{ 0, 0, 0, 255 };

		Text(SDL_Renderer* renderer, std::string_view str, std::string_view fontPath, int fontSize, TTF_Font* font);
		Text(SDL_Renderer* renderer, const json& json, FontMap& fontMap);

		void operator=(std::string_view str) noexcept;

		Text(Text&&)            = default;
		Text& operator=(Text&&) = default;

		std::string_view value() const noexcept;

		void move(int dx, int dy) noexcept;
		void move(SDL_Point p) noexcept;
		void setPos(int x, int y);
		void setPos(SDL_Point p);
		void scale(int dw, int dh) noexcept;
		void scale(SDL_Point p) noexcept;
		bool containsCoord(int x, int y) const noexcept;
		bool containsCoord(SDL_Point p) const noexcept;

		void render() const noexcept;

		void save(json& json) const;
	};

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