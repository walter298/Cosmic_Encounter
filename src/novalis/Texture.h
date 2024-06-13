#pragma once

#include <string_view>

#include "Rect.h"

namespace nv {
	struct Texture {
		SDL_Texture* raw = nullptr;

		Texture() = default;
		explicit Texture(SDL_Texture* texture) noexcept;

		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;

		Texture(Texture&&) noexcept = default;
		Texture& operator=(Texture&&) noexcept = default;

		~Texture() noexcept;
	};

	using TexturePtr = std::shared_ptr<Texture>;

	struct TexturePos {
		static constexpr std::string_view renJkey = "ren";
		static constexpr std::string_view worldJkey = "world";
		static constexpr std::string_view angleJkey = "angle";
		static constexpr std::string_view rotationPointJkey = "rotation_point";
		static constexpr std::string_view flipJkey = "flip";

		Rect ren;
		Rect world;
		SDL_Point rotationPoint{ 0, 0 };
		double angle = 0.0;
		SDL_RendererFlip flip = SDL_FLIP_NONE;
	};

	struct TextureData {
		TexturePtr tex;
		TexturePos pos;
		
		void setOpacity(Uint8 opacity) noexcept;

		void setPos(int x, int y) noexcept;
		void setPos(SDL_Point pos) noexcept;

		SDL_Point getPos() const noexcept;

		void move(int dx, int dy) noexcept;
		void move(SDL_Point change) noexcept;

		void scale(int dx, int dy) noexcept;
		void scale(SDL_Point change) noexcept;

		void rotate(double angle, SDL_Point rotationPoint) noexcept;
		void setRotationCenter() noexcept;

		bool containsCoord(int x, int y) const noexcept;
		bool containsCoord(SDL_Point p) const noexcept;

		void render(SDL_Renderer* renderer) const noexcept;
	};

	/*void from_json(const json& j, TexturePos& pos);
	void to_json(json& j, const TexturePos& pos);*/
}