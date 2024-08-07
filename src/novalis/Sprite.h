#pragma once

#include <fstream>
#include <string_view>

#include <SDL2/SDL_image.h>

#include "DataUtil.h"
#include "Texture.h"

namespace nv {
	struct Scene;
	namespace editor { 
		class SceneEditor; 
		class SpriteEditor;
	}

	class Sprite : public NamedObject {
	private:
		Layers<TextureObject> m_texObjLayers;
		int m_currLayer = 0;
	public:
		Sprite(SDL_Renderer* renderer, const json& json, TextureMap& texMap);

		using JsonFormat = Layers<std::pair<std::string, TextureData>>;

		TextureData& getTexData(size_t texIdx);

		void setPos(int destX, int destY) noexcept;
		void setPos(SDL_Point p) noexcept;

		void move(int x, int y) noexcept;
		void move(SDL_Point p) noexcept;

		void scale(int x, int y) noexcept;
		void scale(SDL_Point p) noexcept;

		void rotate(double angle, SDL_Point p);
		void setRotationCenter() noexcept;

		bool containsCoord(int x, int y) const noexcept;
		bool containsCoord(SDL_Point p) const noexcept;

		void setOpacity(Uint8 opacity);

		void render() const noexcept;

		void save(json& json) const;

		friend class editor::SceneEditor;
		friend class editor::SpriteEditor;
	};
}