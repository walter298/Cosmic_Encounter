#ifndef RENDERER_H
#define RENDERER_H

#define _SILENCE_CXX23_ALIGNED_STORAGE_DEPRECATION_WARNING

#include <chrono>
#include <map>
#include <string>
#include <filesystem>
#include <iostream>

#include <boost/container/flat_map.hpp>

#include <plf_hive.h>

#include <imgui.h>
#include <imgui_impl_sdlrenderer2.h>

#include "Sprite.h"
#include "GlobalMacros.h"

namespace nv {
	class Renderer {
	private:
		SDL_Renderer* m_renderer;
		
		using Layers = FlatOrderedMap<int, plf::hive<Sprite*>>;
		Layers m_objects;
	public:
		Renderer(SDL_Renderer* renderer); 
		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&)      = delete;

		inline SDL_Renderer* get() {
			return m_renderer;
		}

		void clear() noexcept;

		void addObj(Sprite* obj, int layer);
		void removeObj(const ID& ID, int layer);

		void render() noexcept; 
		void render(ImGuiIO& io);
	};
}

#endif