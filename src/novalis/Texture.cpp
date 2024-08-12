#include "Texture.h"

#include <print>

#include "data_util/BasicJsonSerialization.h"
#include "data_util/File.h"

nv::TextureRAII nv::loadTexture(SDL_Renderer* renderer, std::string_view texPath) noexcept {
	return TextureRAII{ IMG_LoadTexture(renderer, texPath.data()), SDL_DestroyTexture };
}

nv::SharedTexture nv::loadSharedTexture(SDL_Renderer* renderer, std::string_view texPath) noexcept {
	return SharedTexture{ IMG_LoadTexture(renderer, texPath.data()), SDL_DestroyTexture };
}

nv::Texture::Texture(SDL_Renderer* renderer, std::string_view texPath, SharedTexture texPtr, TextureData texData) 
	: m_renderer{ renderer }, m_texPath { std::make_shared<std::string>(convertFullToRegularPath(texPath)) },
	m_texVariant{ std::move(texPtr) }, texData{ std::move(texData) }
{
	m_tex = std::get<SharedTexture>(m_texVariant).get();
}

nv::Texture::Texture(SDL_Renderer* renderer, std::string_view texPath, SDL_Texture* rawTex, TextureData texData)
	: m_renderer{ renderer }, m_texPath{ std::make_shared<std::string>(convertFullToRegularPath(texPath)) }, m_texVariant{ std::move(rawTex) }, texData{ std::move(texData) }
{
	m_tex = rawTex;
}

nv::Texture::Texture(SDL_Renderer* renderer, const json& json, TextureMap& texMap) 
	: m_renderer{ renderer } 
{
	auto texPath = relativePath(json["texture_path"].get<std::string>());
	auto texPathIt = texMap.find(texPath);
	if (texPathIt != texMap.end()) {
		m_tex = texPathIt->second.get();
	} else {
		m_tex = IMG_LoadTexture(renderer, texPath.c_str());
		texMap.emplace(std::piecewise_construct, std::forward_as_tuple(std::move(texPath)), std::forward_as_tuple(m_tex, SDL_DestroyTexture));
	}
	texData = json["texture_object_data"].get<TextureData>();
	if (json.contains("name")) {
		m_name = json["name"].get<std::string>();
	}

	std::println("{}", SDL_GetError());
}

const std::string& nv::Texture::getTexPath() const noexcept {
	return *m_texPath;
}

void nv::Texture::setOpacity(Uint8 opacity) noexcept {
	SDL_SetTextureAlphaMod(m_tex, opacity);
}

void nv::Texture::setPos(int x, int y) noexcept {
	texData.ren.setPos(x, y);
	texData.world.setPos(x, y);
}

void nv::Texture::setPos(SDL_Point pos) noexcept {
	setPos(pos.x, pos.y);
}

SDL_Point nv::Texture::getPos() const noexcept {
	return SDL_Point{ texData.ren.rect.x, texData.ren.rect.y };
}

void nv::Texture::move(int dx, int dy) noexcept {
	texData.ren.move(dx, dy);
	texData.world.move(dx, dy);
}

void nv::Texture::move(SDL_Point change) noexcept {
	move(change.x, change.y);
}

void nv::Texture::setSize(int w, int h) noexcept {
	texData.ren.setSize(w, h);
	texData.world.setSize(w, h);
}

void nv::Texture::setSize(SDL_Point p) {
	setSize(p.x, p.y);
}

SDL_Point nv::Texture::getSize() const noexcept {
	return { texData.ren.rect.w, texData.ren.rect.h };
}

void nv::Texture::scale(int dx, int dy) noexcept {
	texData.ren.scale(dx, dy);
	texData.world.scale(dx, dy);
}

void nv::Texture::scale(SDL_Point change) noexcept {
	scale(change.x, change.y);
}

void nv::Texture::rotate(double angle, SDL_Point rotationPoint) noexcept {
	texData.angle = angle;
	texData.rotationPoint = rotationPoint;
}

void nv::Texture::render() const noexcept {
	SDL_RenderCopyEx(m_renderer, m_tex, nullptr, &texData.ren.rect, texData.angle, &texData.rotationPoint, texData.flip);
}

void nv::Texture::save(json& json) const {
	json["texture_path"] = *m_texPath;
	json["texture_object_data"] = texData;
}

void nv::Texture::setRotationCenter() noexcept {
	texData.rotationPoint = {
		(texData.ren.rect.w) / 2,
		(texData.ren.rect.h) / 2
	};
}

bool nv::Texture::containsCoord(int x, int y) const noexcept {
	return texData.ren.containsCoord(x, y);
}

bool nv::Texture::containsCoord(SDL_Point p) const noexcept {
	return containsCoord(p.x, p.y);
}
