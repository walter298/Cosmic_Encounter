#ifndef SCENE_H
#define SCENE_H

#include <nlohmann/json.hpp>

#include "EventHandler.h"

#include "Renderer.h"

#include "Button.h"
#include "Sprite.h"
#include "Text.h"
#include "Texture.h"

#include <print>

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

		Layers<std::reference_wrapper<Sprite>> spriteRefs;
		Layers<std::reference_wrapper<TextureObject>> textureRefs;
		Layers<std::reference_wrapper<Text>> textRefs;
		Layers<std::reference_wrapper<Rect>> rectRefs;

		EventHandler eventHandler;
		
		Scene(std::string_view path, SDL_Renderer* renderer, TextureMap& texMap, FontMap& fontMap);

		void operator()();

		void overlay(Scene& scene);
		void deoverlay();

		auto& find(auto& objs, std::string_view name) {
			auto objIt = ranges::find_if(objs, [&](const auto& obj) { return obj.getName() == name; });
			assert(objIt != objs.end());
			return *objIt;
		}
		auto& findRef(auto& objs, std::string_view name) {
			auto objIt = ranges::find_if(objs, [&](const auto& obj) { return obj.get().getName() == name; });
			if (objIt == objs.end()) {
				for (const auto& obj : objs) {
					std::println("{}", obj.get().getName());
				}
			}
			assert(objIt != objs.end());
			return objIt->get();
		}
	};
}

#endif