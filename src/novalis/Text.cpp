#include "Text.h"

#include "data_util/File.h"

#include <print>

nv::FontRAII nv::loadFont(std::string_view fontPath, int fontSize) {
	return FontRAII{ TTF_OpenFont(fontPath.data(), fontSize), TTF_CloseFont };
}

void nv::Text::changeText(std::string_view newText) noexcept {
	m_str = newText;
	auto surface = TTF_RenderText_Solid(m_font, m_str.data(), { 0, 0, 0, 255 });
	if (surface == nullptr) {
		m_ren.setSize(0, 0);
		return;
	}
	auto w = surface->w;
	auto h = surface->h;

	if (m_tex != nullptr) {
		m_tex.release();
	}
	m_tex.reset(SDL_CreateTextureFromSurface(m_renderer, surface));
	m_ren.setSize(w, h);
	SDL_FreeSurface(surface);
}

nv::Text::Text(SDL_Renderer* renderer, std::string_view str, std::string_view fontPath, int fontSize, TTF_Font* font)
	: m_renderer{ renderer }, m_font{ font }, m_str{ str }, m_fontPath{ fontPath }, m_fontSize{ fontSize }
{
	changeText(str);
}

nv::Text::Text(SDL_Renderer* renderer, const json& json, FontMap& fontMap) 
	: m_renderer{ renderer }
{
	m_fontPath = relativePath(json["font_path"].get<std::string>());
	m_fontSize = json["font_size"].get<int>();

	auto fontName = m_fontPath + std::to_string(m_fontSize);
	auto fontPathIt = fontMap.find(fontName);
	if (fontPathIt == fontMap.end()) {
		auto font = loadFont(m_fontPath, m_fontSize);
		m_font = font.get();
		fontMap.emplace(fontName, std::move(font));
	} else {
		m_font = fontPathIt->second.get();
	}
	m_str = json["value"].get<std::string>();
	changeText(m_str);
	m_ren = json["ren"].get<Rect>();
	m_name = json["name"].get<std::string>();
}

void nv::Text::operator=(std::string_view str) noexcept {
	changeText(str);
}

std::string_view nv::Text::value() const noexcept {
	return m_str;
}

void nv::Text::move(int dx, int dy) noexcept {
	m_ren.move(dx, dy);
}

void nv::Text::move(SDL_Point p) noexcept {
	move(p.x, p.y);
}

void nv::Text::setPos(int x, int y) {
	m_ren.setPos(x, y);
}

void nv::Text::setPos(SDL_Point p) {
	setPos(p.x, p.y);
}

void nv::Text::scale(int dw, int dh) noexcept {
	m_ren.scale(dw, dh);
	std::println("{} {}", m_ren.rect.w, m_ren.rect.h);
}

void nv::Text::scale(SDL_Point p) noexcept {
	scale(p.x, p.y);
}

bool nv::Text::containsCoord(int x, int y) const noexcept {
	return m_ren.containsCoord(x, y);
}

bool nv::Text::containsCoord(SDL_Point p) const noexcept {
	return containsCoord(p.x, p.y);
}

void nv::Text::setOpacity(uint8_t a) noexcept {
	color.a = a;
}

void nv::Text::render() const noexcept {
	SDL_RenderCopy(m_renderer, m_tex.get(), nullptr, &m_ren.rect);
}

void nv::Text::save(json& json) const {
	json["value"]     = m_str;
	json["font_path"] = m_fontPath;
	json["font_size"] = m_fontSize;
	json["ren"]       = m_ren;
}

bool nv::TextInput::tooSoonToPop() const noexcept {
	static constexpr auto MAX_EDIT_WAIT_TIME = 150ms;
	return chrono::system_clock::now() - m_lastTimePopped < MAX_EDIT_WAIT_TIME;
}

nv::TextInput::TextInput(const Rect& rect, Text& text)
	: m_rect{ rect }, m_text{ text }
{
	m_buff.reserve(100);
}

const nv::Rect& nv::TextInput::getRect() const noexcept {
	return m_rect;
}

void nv::TextInput::append(std::string_view inputText) {
	m_buff.append(inputText);
	m_text.get() = m_buff;
}

void nv::TextInput::pop() {
	if (tooSoonToPop()) {
		return;
	}
	auto textValue = m_text.get().value();
	if (!textValue.empty()) {
		m_text.get() = textValue.substr(0, textValue.size() - 1);
		m_buff.pop_back();
	}
	m_lastTimePopped = chrono::system_clock::now();
}