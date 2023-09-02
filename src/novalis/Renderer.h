#ifndef RENDERER_H
#define RENDERER_H

#include <chrono>
#include <map>
#include <string>
#include <filesystem>
#include <iostream>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_sdlrenderer2.h"

#include "RenderTypes.h"
#include "GlobalMacros.h"

namespace nv {
	class Renderer {
	private:
		SDL_Renderer* m_renderer;

		std::vector<std::pair<int, RenderObj*>> m_objects;
	public:
		Renderer(SDL_Renderer* renderer); 
		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&)      = delete;

		inline SDL_Renderer* get() {
			return m_renderer;
		}

		void clear() noexcept;

		void addObj(RenderObj* obj, int layer) noexcept;
		void removeObj(ID<RenderObj> ID);

		void render() noexcept; 
		void renderImgui(ImGuiIO& io);
	};
}

#endif