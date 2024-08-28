#include "Rect.h"

#include "data_util/BasicJsonSerialization.h"

bool nv::Rect::isInRegion(int mx, int my, int x, int y, int w, int h) noexcept {
	return mx > x && mx < x + w &&
		my > y && my < y + h;
}

bool nv::Rect::isInRegion(SDL_Point coord, int x, int y, int w, int h) noexcept {
	return isInRegion(coord.x, coord.y, x, y, w, h);
}

nv::Rect::Rect(SDL_Renderer* renderer) noexcept
	: renderer{ renderer }
{
}

nv::Rect::Rect(SDL_Renderer* renderer, int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	: renderer{ renderer }, rect{ x, y, w, h }, color{ r, g, b, a }
{
}

nv::Rect::Rect(SDL_Renderer* renderer, const json& json) {
	*this = json.get<Rect>();
	this->renderer = renderer;
}

bool nv::Rect::containsCoord(int mx, int my) const noexcept {
	return isInRegion(mx, my, rect.x, rect.y, rect.w, rect.h);
}

bool nv::Rect::containsCoord(SDL_Point p) const noexcept {
	return containsCoord(p.x, p.y);
}

void nv::Rect::move(int dx, int dy) noexcept {
	rect.x += dx;
	rect.y += dy;
}
void nv::Rect::move(SDL_Point p) noexcept {
	move(p.x, p.y);
}
void nv::Rect::scale(int dw, int dh) noexcept {
	rect.w += dw;
	rect.h += dh;
}
void nv::Rect::scale(SDL_Point p) noexcept {
	scale(p.x, p.y);
}
void nv::Rect::setPos(int x, int y) noexcept {
	rect.x = x;
	rect.y = y;
}
void nv::Rect::setPos(SDL_Point p) noexcept {
	setPos(p.x, p.y);
}
SDL_Point nv::Rect::getPos() const noexcept {
	return SDL_Point{ rect.x, rect.y };
}
void nv::Rect::setSize(int w, int h) noexcept {
	rect.w = w;
	rect.h = h;
}
void nv::Rect::setSize(SDL_Point p) noexcept
{
	setSize(p.x, p.y);
}

SDL_Point nv::Rect::getSize() const noexcept {
	return { rect.w, rect.h };
}

void nv::Rect::setOpacity(uint8_t a) {
	color.a = a;
}

void nv::Rect::render() const noexcept {
	assert(renderer != nullptr);

	if (color.a == 0) {
		return;
	}

	//get original render data
	Uint8 r; Uint8 b; Uint8 g; Uint8 a;
	SDL_GetRenderDrawColor(renderer, &r, &b, &g, &a); 
	SDL_BlendMode originalBlendMode;
	SDL_GetRenderDrawBlendMode(renderer, &originalBlendMode);

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	SDL_RenderFillRect(renderer, &rect);

	//go back to original renderer state
	SDL_SetRenderDrawColor(renderer, r, b, g, a);
	SDL_SetRenderDrawBlendMode(renderer, originalBlendMode);
}

void nv::Rect::setRenderColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) noexcept {
	std::tie(color.r, color.g, color.b, color.a) = std::tie(r, g, b, a);
}

void nv::Rect::save(json& json) const {
	json = *this;
}

void nv::to_json(json& j, const Rect& r) {
	j["sdl_rect"] = r.rect;
	j["color"]    = r.color;
	j["name"] = r.getName();
}

void nv::from_json(const json& j, Rect& r) {
	r.rect = j.at("sdl_rect").get<SDL_Rect>();
	r.color = j.at("color").get<SDL_Color>();
	if (j.contains("name")) {
		r.m_name = j.at("name").get<std::string>();
	}
}