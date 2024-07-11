#ifndef SCENE_H
#define SCENE_H

#include <nlohmann/json.hpp>

#include "EventHandler.h"

#include "Renderer.h"

#include "Button.h"
#include "Sprite.h"
#include "Text.h"
#include "Texture.h"

namespace nv {
	class Scene {
	private:
		SDL_Renderer* m_renderer;
		FontMap m_fontMap;
		TextureMap m_texMap;
	public:
		bool running = false;

		Layers<Sprite> sprites;
		Layers<TextureObject> textures;
		Layers<Text> text;
		Layers<Rect> rects;

		EventHandler eventHandler;
		
		Scene(std::string_view path, SDL_Renderer* renderer);

		void operator()();
	};
}

#endif