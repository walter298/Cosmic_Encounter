#ifndef INSTANCE_H
#define INSTANCE_H

#include <any>

#include "Renderer.h"
#include "Sprite.h"

#include "Sound.h"

namespace nv {
	class Instance {
	private:
		SDL_Window* m_SDLWindow;
		SDL_Renderer* m_SDLRenderer;

		using SpriteMap = std::unordered_map<std::string, Sprite>;
		SpriteMap m_spriteMap;

		//std::string param is the filepath
		using ObjLoader = std::function<std::pair<std::string, std::any>(std::string)>;

		std::unordered_map<std::string, ObjLoader> m_typeLoaders;
		std::unordered_map<std::string, std::any> m_customTypeMap;

		void quit();

		Instance() = default;
	public:
		Instance(std::string windowTitle);
		~Instance();

		SDL_Window* getRawWindow() noexcept;
		SDL_Renderer* getRawRenderer() noexcept;

		Sprite& getSprite(const std::string& name);
		
		void setCustomObjLoader(std::string fileExt, ObjLoader loader);

		template<typename T>
		T getCustomObj(std::string name) {
			return std::any_cast<T>(m_customTypeMap.at(name));
		}

		void loadObjsFromDir(std::string absDirPath);
	};
}

#endif