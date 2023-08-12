#pragma once

#include "Renderer.h"
#include "RenderTypes.h"

#include "Sound.h"

namespace nv {
	struct NovalisInstance {
	private:
		SDL_Window* m_window;
		SDL_Renderer* m_SDLRenderer;

		std::map<std::string, RenderObjPtr> m_objMap;

		Renderer m_renderer;

		void quit();

		NovalisInstance() = default;
	public:
		NovalisInstance(std::string windowTitle);
		~NovalisInstance();

		inline SDL_Window* window() noexcept {
			return m_window;
		}

		inline Renderer& renderer() noexcept {
			return m_renderer;
		}

		template<typename Obj>
		Obj* getObj(std::string name) 
			requires(std::is_base_of_v<RenderObj, Obj>) 
		{
			return dynamic_cast<Obj*>(m_objMap.at(name).get());
		}

		void loadObj(std::string absFilePath);
		void loadObjsFromDir(std::string absDirPath);

		void removeObj(std::string name);
	};
}