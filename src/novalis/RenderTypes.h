#ifndef Sprite_H
#define Sprite_H

#include <fstream>
#include <filesystem>
#include <iostream>
#include <memory>
#include <ranges>
#include <string>
#include <vector>

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"

#include "Rect.h"
#include "DataUtil.h"
#include "ID.h"

namespace nv {
	class RenderObj {
	private:
		ID<RenderObj> m_ID;
	protected:
		std::string m_name;
	public:
		const std::string& getName() const noexcept;

		ID<RenderObj> getID() const noexcept;

		virtual void render(SDL_Renderer* renderer) noexcept = 0;
	};

	template<DerivedFrom<RenderObj> T>
	bool operator==(const T& t, const T& u) noexcept {
		return t.getID() == u.getID();
	}

	using RenderObjPtr = std::unique_ptr<RenderObj>;

	struct Texture {
		SDL_Texture* raw = nullptr;

		explicit Texture(SDL_Texture* texture) noexcept;

		Texture(const Texture&)            = delete;
		Texture& operator=(const Texture&) = delete;

		Texture(Texture&&) noexcept            = default;
		Texture& operator=(Texture&&) noexcept = default;

		~Texture() noexcept;
	};

	using TexturePtr = std::shared_ptr<Texture>;
	
	class Sprite final : public RenderObj {
	private:
		std::vector<TexturePtr> m_spriteSheet;

		size_t m_spriteIndex = 0;

		SDL_Texture* m_currentSprite = nullptr;

		using CollisionBoxes = std::vector<std::vector<Rect>>;
		using RenderBoxes    = std::vector<Rect>;

		CollisionBoxes m_collisionBoxes;
		std::vector<Rect> m_renderBoxes;

		Rect m_ren, m_world;
	public:
		Sprite() = default;
		Sprite(SDL_Renderer* renderer, std::string path);
		
		inline SDL_Texture* getSprite() {
			return m_currentSprite;
		}

		inline Rect& getRen() {
			return m_ren;
		}

		inline Rect& getWorld() {
			return m_world;
		}

		inline void renMove(int dx, int dy) noexcept {
			m_ren.move(dx, dy);
		}

		inline void worldMove(int dx, int dy) noexcept {
			m_world.move(dx, dy);
		}

		void move(int dx, int dy) noexcept;

		inline void setRenPos(int x, int y) noexcept {
			m_ren.setPos(x, y);
		}

		inline void setWorldPos(int x, int y) noexcept {
			m_world.setPos(x, y);
		}

		inline void renScale(int dw, int dh) noexcept {
			m_ren.scale(dw, dh);
		}

		inline void worldScale(int dw, int dh) noexcept {
			m_world.scale(dw, dh);
		}

		inline void setRenSize(int w, int h) noexcept {
			m_ren.setSize(w, h);
		}

		inline void setWorldSize(int w, int h) noexcept {
			m_world.setSize(w, h);
		}

		inline void scale(int dw, int dh) noexcept {
			renScale(dw, dh);
			worldScale(dw, dh);
		}

		inline void changeTexture(size_t idx) {
			m_currentSprite = m_spriteSheet[idx]->raw;
			m_ren = m_renderBoxes[idx];
		}

		void render(SDL_Renderer* renderer) noexcept override;
	};

	struct Background final : public RenderObj {
	private:
		std::vector<nv::Rect> m_rens;
		std::vector<TexturePtr> m_backgrounds;
	public:
		Background() = default;
		Background(SDL_Renderer* renderer, std::string absFilePath);

		void render(SDL_Renderer* renderer) noexcept override;

		void renMove(int dx, int dy) noexcept;
	};

	using SpritePtr = std::unique_ptr<Sprite>;

	enum class FontType {
		WorkSans,
		Libertine,
	};

	class Text;

	template<typename Stream>
	Stream& operator<<(Stream& stream, const Text& text);

	class Text final : public RenderObj {
	private:
		TexturePtr m_tex;

		FontType m_fontType;
		std::string m_text;
		SDL_Color m_color{ 255, 255, 255, 0 };
		Uint32 m_wrapLength = 0;

		nv::Rect m_background{ 0, 0, 0, 0 };
		nv::Rect m_ren{ 0, 0, 0, 0 };

		static inline std::map<FontType, TTF_Font*> fonts;

		void init(SDL_Renderer* renderer);
	public:
		static void loadFonts();
		static void closeFonts();

		Text() = default;
		Text(SDL_Renderer* renderer, std::string absPath);
		Text(SDL_Renderer* renderer, std::string text, FontType font, SDL_Color color, Uint32 wrapLength);

		inline Rect& backgroundRect() noexcept {
			return m_background;
		}

		inline const std::string& text() const noexcept {
			return m_text;
		}

		virtual void render(SDL_Renderer* renderer) noexcept override;

		void setRenPos(int x, int y);
		void setSize(int w, int h);
		
		void setText(SDL_Renderer* renderer, std::string text);
		void setFont(SDL_Renderer* renderer, FontType font);
		void setColor(SDL_Renderer* rendererer, SDL_Color color);
		void setWrapLength(SDL_Renderer* renderer, Uint32 color);

		template<typename Stream>
		friend Stream& operator<<(Stream& stream, const Text& text);
	};

	using TextPtr = std::unique_ptr<Text>;

	template<typename Stream> 
	Stream& operator<<(Stream& stream, const Text& text) {
		json j;
		j["type"]        = "text"s;
		j["name"]        = text.m_name;
		j["text"]        = text.m_text;
		j["color"]       = text.m_color;
		j["wrap_length"] = text.m_wrapLength;
		j["background"]  = text.m_background;
		j["ren"]         = text.m_ren;
		stream << j.dump(2);
		return stream;
	}
}

namespace std {
	template<nv::DerivedFrom<nv::RenderObj> T>
	struct hash<T> {
		std::size_t operator()(const T& obj) const {
			return std::hash<int>{}(obj.getID().raw());
		}
	};
}

#endif