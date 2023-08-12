#ifndef Sprite_H
#define Sprite_H

#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <memory>

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"

#include "Rect.h"
#include "DataUtil.h"

namespace nv {
	template<typename Obj>
	std::unique_ptr<Obj> copyRenderObj(const Obj* other);

	class RenderObj {
	protected:
		static int IDCount;

		std::string m_name;
		int m_ID = 0;
		int m_layer = 0;

		RenderObj() noexcept;
	public:
		const std::string& getName() const noexcept;

		int getID() const noexcept;
		int getLayer() const noexcept;

		virtual void render(SDL_Renderer* renderer) = 0;
		virtual void move(int dx, int dy) = 0;

		template<typename Obj>
		friend std::unique_ptr<Obj> copyRenderObj(const Obj* other);
	};

	using RenderObjPtr = std::unique_ptr<RenderObj>;

	struct Texture {
		SDL_Texture* raw = nullptr;
	
		explicit Texture(SDL_Texture* texture) noexcept;
		Texture(const Texture&) = delete;
		~Texture() noexcept;
	};

	using TexturePtr = std::shared_ptr<Texture>;

	inline constexpr int BACKGROUND_LAYER = 0;

	class Sprite final : public RenderObj {
	private:
		std::vector<TexturePtr> m_spriteSheet;

		size_t m_spriteIndex = 0;

		SDL_Texture* m_currentSprite = nullptr;

		using CollisionBoxes = std::vector<std::vector<Rect>>;
		using RenderBoxes    = std::vector<Rect>;

		CollisionBoxes m_collisionBoxes;
		RenderBoxes    m_renderBoxes;

		Rect m_ren, m_world;

		int m_layer = 0; //layer of screen rendering
	public:
		Sprite(SDL_Renderer* Renderer, std::string path);
		Sprite(const Sprite&) = delete; //prevent slicing

		inline SDL_Texture* getSprite() {
			return m_currentSprite;
		}

		inline Rect& getRen() {
			return m_ren;
		}

		inline Rect& getWorld() {
			return m_world;
		}

		inline void renMove(int dx, int dy) {
			m_ren.move(dx, dy);
		}

		inline void worldMove(int dx, int dy) {
			m_world.move(dx, dy);
		}

		inline void move(int dx, int dy) {
			renMove(dx, dy);
			worldMove(dx, dy);
		}

		inline void setRenPos(int x, int y) {
			m_ren.setPos(x, y);
		}

		inline void setWorldPos(int x, int y) {
			m_world.setPos(x, y);
		}

		inline void renScale(int dw, int dh) {
			m_ren.scale(dw, dh);
		}

		inline void worldScale(int dw, int dh) {
			m_world.scale(dw, dh);
		}

		inline void setSize(int w, int h) {
			m_ren.setSize(w, h);
		}

		inline void scale(int dw, int dh) {
			renScale(dw, dh);
			worldScale(dw, dh);
		}

		inline void changeTexture(size_t idx) {
			m_currentSprite = m_spriteSheet[idx]->raw;
			m_ren = m_renderBoxes[idx];
		}

		inline virtual void render(SDL_Renderer* renderer) noexcept override {
			SDL_RenderCopy(renderer, m_currentSprite, nullptr, &m_ren.rect);
		}
	};

	struct Background final : public RenderObj {
	private:
		TexturePtr m_tex;
		Rect m_ren;
	public:
		Background(SDL_Renderer* renderer, std::string data);
		virtual void render(SDL_Renderer* renderer) override;
		virtual void move(int dx, int dy) override;
	};

	using SpritePtr  = std::unique_ptr<Sprite>;

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

		Text(SDL_Renderer* renderer, std::string absPath);
		Text(SDL_Renderer* renderer, std::string text, FontType font, SDL_Color color, Uint32 wrapLength);
		
		inline Rect& backgroundRect() {
			return m_background;
		}

		inline const std::string& text() {
			return m_text;
		}

		virtual void render(SDL_Renderer* renderer) noexcept override;

		void setRenPos(int x, int y);
		void setSize(int w, int h);
		virtual void move(int dx, int dh) override;

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
		writeSection(stream, "TEXT", text.m_text, 
			writeNums(text.m_color.r, text.m_color.g, text.m_color.b, text.m_color.a), text.m_wrapLength);

		const Rect& ren = text.m_ren, background = text.m_background;

		writeSection(stream, "RENDER_DIMENSIONS", writeNums(ren.rect.w, ren.rect.h));

		writeSection(stream, "BACKGROUND",
			writeNums(background.rect.x - ren.rect.x , background.rect.y - ren.rect.y, background.rect.w, background.rect.h),
			writeNums(background.color.r, background.color.g, background.color.b, background.color.a)
		);
		
		return stream;
	}

	template<typename Obj> 
	std::unique_ptr<Obj> copyRenderObj(const Obj* other) 
		//requires(std::is_base_of_v<RenderObj, Obj>) 
	{
		auto ret = std::make_unique<Obj>(*other);
		
		RenderObj::IDCount++;
		ret->m_ID = RenderObj::IDCount;

		return ret;
	}
}

#endif