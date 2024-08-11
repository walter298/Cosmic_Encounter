#pragma once

#include <fstream>
#include <functional>
#include <string_view>
#include <unordered_map>
#include <variant>

#include <SDL2/SDL_image.h>

#include "Rect.h"

namespace nv {
	using TextureRAII = std::unique_ptr<SDL_Texture, void(*)(SDL_Texture*)>;
	TextureRAII loadTexture(SDL_Renderer* renderer, std::string_view texPath) noexcept;

	using TextureMap = std::unordered_map<std::string, TextureRAII>;

	using SharedTexture = std::shared_ptr<SDL_Texture>;
	SharedTexture loadSharedTexture(SDL_Renderer* renderer, std::string_view texPath) noexcept;

	struct TextureData {
		Rect ren;
		Rect world;
		SDL_Point rotationPoint{ 0, 0 };
		double angle = 0.0;
		SDL_RendererFlip flip = SDL_FLIP_NONE;
	};

	class Texture : public NamedObject {
	private:
		SDL_Renderer* m_renderer;
		std::variant<SharedTexture, SDL_Texture*> m_texVariant;
		SDL_Texture* m_tex = nullptr;
		std::shared_ptr<const std::string> m_texPath = nullptr;
	public:
		Texture() = default;
		Texture(SDL_Renderer* renderer, std::string_view texPath, SharedTexture texPtr, TextureData texData);
		Texture(SDL_Renderer* renderer, std::string_view texPath, SDL_Texture* rawTex, TextureData texData);
		Texture(SDL_Renderer* renderer, const json& json, TextureMap& texMap);
		
		const std::string& getTexPath() const noexcept;

		TextureData texData;
		
		void setOpacity(Uint8 opacity) noexcept;

		void setPos(int x, int y) noexcept;
		void setPos(SDL_Point pos) noexcept;

		SDL_Point getPos() const noexcept;

		void move(int dx, int dy) noexcept;
		void move(SDL_Point change) noexcept;

		void setSize(int w, int h) noexcept;
		void setSize(SDL_Point p);

		SDL_Point getSize() const noexcept;

		void scale(int dx, int dy) noexcept;
		void scale(SDL_Point change) noexcept;

		void rotate(double angle, SDL_Point rotationPoint) noexcept;
		void setRotationCenter() noexcept;

		bool containsCoord(int x, int y) const noexcept;
		bool containsCoord(SDL_Point p) const noexcept;

		void render() const noexcept;

		void save(json& json) const;
	};

	using TextureRef = std::reference_wrapper<Texture>;
}
