#pragma once

#include <fstream>
#include <string_view>
#include <optional>

#include <SDL2/SDL_image.h>

#include "data_util/BasicConcepts.h"
#include "Texture.h"

namespace nv {
	namespace editor { 
		class SceneEditor; 
		class SpriteEditor;
	}

	class Sprite : public ObjectBase<Sprite> {
	private:
		boost_con::flat_map<int, std::vector<Texture>> m_texObjLayers;
		int m_currLayer = 0;
	public:
		Sprite(SDL_Renderer* renderer, const json& json, TextureMap& texMap);

		using JsonFormat = boost_con::flat_map<int, std::vector<std::pair<std::string, TextureData>>>;

		TextureData& getTexData(size_t texIdx);
		void setTextureLayer(int layer) noexcept;

		const std::vector<nv::Texture>& getTextures() const noexcept;

		void setPos(int destX, int destY) noexcept;
		void setPos(SDL_Point p) noexcept;

		void move(int x, int y) noexcept;
		void move(SDL_Point p) noexcept;

		void scale(int x, int y) noexcept;
		void scale(SDL_Point p) noexcept;

		void rotate(double angle, SDL_Point p);
		void setRotationCenter() noexcept;

		std::optional<size_t> containsCoord(int x, int y) const noexcept;
		std::optional<size_t> containsCoord(SDL_Point p) const noexcept;

		void setOpacity(Uint8 opacity);

		void render() const noexcept;

		void save(json& json) const;

		friend class editor::SceneEditor;
		friend class editor::SpriteEditor;
	};

	using SpriteRef = std::reference_wrapper<Sprite>;
}