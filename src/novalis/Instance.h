#pragma once

#include "Renderer.h"
#include "RenderTypes.h"

#include "Sound.h"

namespace nv {
	class Instance {
	private:
		SDL_Window* m_SDLWindow;
		SDL_Renderer* m_SDLRenderer;

		std::map<std::string, Sprite> m_spriteMap;
		std::map<std::string, Background> m_backgroundMap;
		std::map<std::string, Text> m_textMap;

		void quit();

		Instance() = default;
	public:
		Instance(std::string windowTitle);
		~Instance();

		SDL_Window* getRawWindow() noexcept;
		SDL_Renderer* getRawRenderer() noexcept;

		Sprite getSprite(std::string name) const;
		Background getBackground(std::string name) const;
		Text getText(std::string name) const;

		void loadObjsFromDir(std::string absDirPath);
	};
}