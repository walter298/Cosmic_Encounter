#include "Texture.h"

#include <print>

nv::Texture::Texture(SDL_Texture* tex) noexcept
	: raw(tex) {}

nv::Texture::~Texture() noexcept {
	if constexpr (NV_DEBUGGING) {
		std::println("Destroying Texture\n");
	}
	SDL_DestroyTexture(raw);
}

//void nv::from_json(const json& j, TexturePos& pos) {
//	pos.ren = j.at(TexturePos::renJkey).get<Rect>();
//	pos.world = j.at(TexturePos::worldJkey).get<Rect>();
//	pos.angle = j.at(TexturePos::angleJkey).get<size_t>();
//	pos.rotationPoint = j.at(TexturePos::rotationPointJkey).get<SDL_Point>();
//	pos.flip = j.at(TexturePos::flipJkey).get<SDL_RendererFlip>();
//}
//
//void nv::to_json(json& j, const TexturePos& pos) {
//
//}

void nv::TextureData::setOpacity(Uint8 opacity) noexcept {
	SDL_SetTextureAlphaMod(tex->raw, opacity);
}

void nv::TextureData::setPos(int x, int y) noexcept {
	pos.ren.setPos(x, y);
	pos.world.setPos(x, y);
}

void nv::TextureData::setPos(SDL_Point pos) noexcept {
	setPos(pos.x, pos.y);
}

SDL_Point nv::TextureData::getPos() const noexcept {
	return SDL_Point{ pos.ren.rect.x, pos.ren.rect.y };
}

void nv::TextureData::move(int dx, int dy) noexcept {
	pos.ren.move(dx, dy);
	pos.world.move(dx, dy);
}

void nv::TextureData::move(SDL_Point change) noexcept {
	move(change.x, change.y);
}

void nv::TextureData::scale(int dx, int dy) noexcept {
	pos.ren.scale(dx, dy);
	pos.world.scale(dx, dy);
}

void nv::TextureData::scale(SDL_Point change) noexcept {
	scale(change.x, change.y);
}

void nv::TextureData::rotate(double angle, SDL_Point rotationPoint) noexcept {
	pos.angle = angle;
	pos.rotationPoint = rotationPoint;
}

void nv::TextureData::render(SDL_Renderer* renderer) const noexcept {
	SDL_RenderDrawPoint(renderer, pos.rotationPoint.x, pos.rotationPoint.y);
	SDL_RenderCopyEx(renderer, tex->raw, nullptr, &pos.ren.rect, pos.angle, &pos.rotationPoint, pos.flip);
}

void nv::TextureData::setRotationCenter() noexcept {
	pos.rotationPoint = {
		(pos.ren.rect.w) / 2,
		(pos.ren.rect.h) / 2
	};
}

bool nv::TextureData::containsCoord(int x, int y) const noexcept {
	return pos.ren.containsCoord(x, y);
}

bool nv::TextureData::containsCoord(SDL_Point p) const noexcept {
	return containsCoord(p.x, p.y);
}
