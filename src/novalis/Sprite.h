#pragma once

#include <fstream>
#include <string_view>

#include <SDL2/SDL_image.h>

#include "DataUtil.h"
#include "ID.h"
#include "Texture.h"

namespace nv {
	namespace editor {
		class SpriteEditor;
	}

	class Sprite {
	private:
		static constexpr std::string_view textureRenPairsJkey = "textures_and_rens";
		static constexpr std::string_view textureCountJkey = "texture_size";
		//static constexpr std::string_view layerIdxsJkey = "layer_idxs";

		using TextureLayers = FlatOrderedMap<int, std::vector<TextureData>>;
		TextureLayers m_textures;
		int m_currLayer = 0;

		Sprite() = default;
	public:
		Sprite(std::string_view path, SDL_Renderer* renderer) noexcept;

		void setLayer(int layer) noexcept; 

		TexturePos& texData(size_t texIdx);

		void move(int x, int y) noexcept;
		void move(SDL_Point p) noexcept;

		void scale(int x, int y) noexcept;
		void scale(SDL_Point p) noexcept;

		void rotate(double angle, SDL_Point p);
		void setRotationCenter() noexcept;

		bool containsCoord(int x, int y) const noexcept;
		bool containsCoord(SDL_Point p) const noexcept;

		void render(SDL_Renderer* renderer) const noexcept;
		
		friend class editor::SpriteEditor;
	};

	using Sprites = std::vector<Sprite>;
}