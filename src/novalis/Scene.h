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
	struct Scene {
		SDL_Renderer* renderer;

		FontMap& fontMap;
		TextureMap& texMap;

		bool running = false;

		Layers<Sprite> sprites;
		Layers<TextureObject> textures;
		Layers<Text> text;
		Layers<Rect> rects;

		EventHandler eventHandler;
		
		Scene(std::string_view path, SDL_Renderer* renderer, TextureMap& texMap, FontMap& fontMap);

		void operator()();
	};
}

#endif