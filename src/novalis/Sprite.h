#ifndef RENDER_TYPES_H
#define RENDER_TYPES_H

#include <fstream>
#include <filesystem>
#include <iostream>
#include <memory>
#include <ranges>
#include <regex>
#include <string>
#include <vector>

#undef min
#undef max

#include <plf_hive.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "Rect.h"
#include "DataUtil.h"
#include "ID.h"

namespace nv {
	struct Texture {
		SDL_Texture* raw = nullptr;

		Texture() = default;
		explicit Texture(SDL_Texture* texture) noexcept;

		Texture(const Texture&)            = delete;
		Texture& operator=(const Texture&) = delete;

		Texture(Texture&&) noexcept            = default;
		Texture& operator=(Texture&&) noexcept = default;

		~Texture() noexcept;
	};

	using TexturePtr = std::shared_ptr<Texture>;
	using TexturePtrs = std::vector<TexturePtr>;
	//class RenderObj {
	//protected:
	//	std::string m_name;
	//	ID m_ID;
	//public:
	//	const std::string& getName() const noexcept;
	//	const ID& getID() const noexcept;
	//	virtual void render(SDL_Renderer* renderer) = 0;
	//};
	//using RenderObjPtr = StackPtr<RenderObj>;

	//class Sprite : public RenderObj {
	//private:
	//	std::vector<TexturePtr> m_spriteSheet;

	//	size_t m_spriteIndex = 0;

	//	SDL_Texture* m_currentSprite = nullptr;

	//	using CollisionBoxes = std::vector<std::vector<Rect>>;
	//	using RenderBoxes    = std::vector<Rect>;

	//	CollisionBoxes m_collisionBoxes;
	//	std::vector<Rect> m_renderBoxes;

	//	Rect m_ren, m_world;
	//public:
	//	Sprite() = default;
	//	Sprite(SDL_Renderer* renderer, std::string path);

	//	inline SDL_Texture* getSprite() {
	//		return m_currentSprite;
	//	}

	//	inline Rect& getRen() {
	//		return m_ren;
	//	}

	//	inline Rect& getWorld() {
	//		return m_world;
	//	}

	//	inline void renMove(int dx, int dy) noexcept {
	//		m_ren.move(dx, dy);
	//	}

	//	inline void worldMove(int dx, int dy) noexcept {
	//		m_world.move(dx, dy);
	//	}

	//	void Coord(int dx, int dy) noexcept;

	//	inline void setRenPos(int x, int y) noexcept {
	//		m_ren.setPos(x, y);
	//	}

	//	inline void setWorldPos(int x, int y) noexcept {
	//		m_world.setPos(x, y);
	//	}

	//	inline void renScale(int dw, int dh) noexcept {
	//		m_ren.scale(dw, dh);
	//	}

	//	inline void worldScale(int dw, int dh) noexcept {
	//		m_world.scale(dw, dh);
	//	}

	//	inline void setRenSize(int w, int h) noexcept {
	//		m_ren.setSize(w, h);
	//	}

	//	inline void setWorldSize(int w, int h) noexcept {
	//		m_world.setSize(w, h);
	//	}

	//	inline void scale(int dw, int dh) noexcept {
	//		renScale(dw, dh);
	//		worldScale(dw, dh);
	//	}

	//	inline void changeTexture(size_t idx) {
	//		m_currentSprite = m_spriteSheet[idx]->raw;
	//		m_ren = m_renderBoxes[idx];
	//	}

	//	void render(SDL_Renderer* renderer) noexcept override;
	//};

	//using SpritePtr = std::unique_ptr<Sprite>;

	//struct Background : public RenderObj {
	//private:
	//	std::vector<nv::Rect> m_rens;
	//	std::vector<TexturePtr> m_backgrounds;
	//public:
	//	Background() = default;
	//	Background(SDL_Renderer* renderer, std::string absFilePath);

	//	void render(SDL_Renderer* renderer) noexcept override;

	//	void renMove(int dx, int dy) noexcept;
	//};

	//enum class FontType {
	//	WorkSans,
	//	Libertine,
	//};

	//using FontMap = std::unordered_map<FontType, TTF_Font*>;

	//namespace editor {
	//	class TextEditor;
	//}

	//class Text : public RenderObj {
	//private:
	//	TexturePtr m_tex = std::make_shared<Texture>(nullptr);
	//	Rect m_rect;
	//	TTF_Font* m_font = nullptr;
	//	std::string m_text;
	//	SDL_Color m_color{ 255, 255, 255, 0 };
	//	int m_fontSize = 14;

	//	nv::Rect m_background{ 0, 0, 0, 0 };
	//	nv::Rect m_ren{ 0, 0, 0, 0 };

	//	static inline FontMap fontMap;
	//public:
	//	//these methods should ONLY be called by nv::Instance
	//	static void openFonts() noexcept;
	//	static void closeFonts() noexcept;

	//	Text() = default; 
	//	Text(SDL_Renderer* renderer, std::string absPath);

	//	virtual void render(SDL_Renderer* renderer) noexcept override;

	//	void setRenPos(int x, int y) noexcept;
	//	void setFontSize(SDL_Renderer* renderer, int fontSize) noexcept;
	//	void changeText(SDL_Renderer* renderer, std::string text) noexcept;
	//	void setColor(SDL_Color color) noexcept;
	//	
	//	friend class editor::TextEditor;
	//};

	enum class Flip {
		None,
		Horizontal,
		Vertical
	};

	//friend classes
	namespace editor {
		class SceneEditor; //friend of Sprite and Background
		class SpriteEditor; //friend of Sprite
		class BackgroundEditor; //friend of Background
	}

	enum class SpriteType {
		Sprite,
		Background
	};

	namespace detail {
		void parseImages(nlohmann::json& j, SDL_Renderer* renderer, TexturePtrs& textures);
	}

	class Sprite {
	private:
		TexturePtrs m_textures;
		ID m_ID;
		std::string m_name;
		size_t m_currTexGroupIdx = 0;
		double m_angle = 0.0;
		SDL_Point m_rotationPoint{ 0, 0 };
		SDL_RendererFlip m_flip = SDL_FLIP_NONE;
	public:
		Sprite() = default;
		Sprite(SDL_Renderer* renderer, const std::string& path, const std::string& name = "");
		
		Rect ren;
		Rect world;

		const ID& getID() const noexcept;
		const std::string& getName() const noexcept;
		
		void changeTexture(size_t texIdx) noexcept;
		void flip(SDL_RendererFlip flip);
		void rotate(double angle, int x, int y) noexcept;
		void render(SDL_Renderer* renderer) const noexcept;

		friend class editor::SceneEditor;
		friend class editor::SpriteEditor;
	};

	using Sprites = plf::hive<Sprite>;

	class Background;

	class Background {
	private:
		TexturePtrs m_textures;
		int m_x = 0;
		int m_y = 0;
		int m_width = 0;
		int m_height = 0;

		int m_horizTexC = 1;
	public:
		nv::Rect ren{ 0, 0, 0, 0 };

		Background() = default;
		Background(SDL_Renderer* renderer, const std::string& path);
		void render(SDL_Renderer* renderer) const noexcept;

		friend class editor::BackgroundEditor;
		friend class editor::SceneEditor;
	};

	/*using TextPtr = std::unique_ptr<Text>;

	template<typename Stream> 
	Stream& operator<<(Stream& stream, const Text& text) {
		json j;
		j["type"]        = "text"s;
		j["name"]        = text.m_name;
		j["text"]        = text.m_text;
		j["color"]       = text.m_color;
		j["background"]  = text.m_background;
		j["ren"]         = text.m_ren;
		stream << j.dump(2);
		return stream;
	}*/
}

#endif