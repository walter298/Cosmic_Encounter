#include "Sprite.h"

nv::Sprite::Sprite(std::string_view path, SDL_Renderer* renderer) noexcept {
	std::ifstream file{ path.data() };
	auto json = json::parse(file);
	file.close();
	
	/*m_textures.reserve(json.at(textureCountJkey).get<size_t>());

	using StringRenPairs = FlatOrderedMap<int, std::vector<std::pair<std::string, TexturePos>>>;
	auto imagePathsAndRens = json.at(textureRenPairsJkey).get<StringRenPairs>();

	for (auto& [layer, textures] : imagePathsAndRens) {
		for (auto& [imagePath, texPos] : textures) {
			m_textures[layer].emplace_back(
				std::make_shared<Texture>(IMG_LoadTexture(renderer, imagePath.c_str())),
				std::move(texPos)
			);
		}
	}*/
}

void nv::Sprite::setLayer(int layer) noexcept {
	m_currLayer = layer;
}

nv::TexturePos& nv::Sprite::texData(size_t texIdx) {
	return m_textures[m_currLayer][texIdx].pos;
}

void nv::Sprite::move(int x, int y) noexcept {
	for (auto& texData : m_textures.at(m_currLayer)) {
		texData.move(x, y);
	}
}

void nv::Sprite::move(SDL_Point p) noexcept {
	move(p.x, p.y);
}

void nv::Sprite::scale(int x, int y) noexcept {
	for (auto& texData : m_textures.at(m_currLayer)) {
		texData.scale(x, y);
	}
}

void nv::Sprite::scale(SDL_Point p) noexcept {
	scale(p.x, p.y);
}

//don't know how the hell this will work
void nv::Sprite::rotate(double angle, SDL_Point p) {}

void nv::Sprite::setRotationCenter() noexcept {}

bool nv::Sprite::containsCoord(int x, int y) const noexcept {
	return ranges::any_of(m_textures.at(m_currLayer), [&](const auto& texData) { 
		return texData.containsCoord(x, y);
	});
}

bool nv::Sprite::containsCoord(SDL_Point p) const noexcept {
	return containsCoord(p.x, p.y);
}

void nv::Sprite::render(SDL_Renderer* renderer) const noexcept {
	if constexpr (NV_DEBUGGING) {
		if (!m_textures.contains(m_currLayer)) {
			return;
		}
	}
	for (const auto& texData : m_textures.at(m_currLayer)) {
		texData.render(renderer);
	}
}