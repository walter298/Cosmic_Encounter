#ifndef RENDERER_H
#define RENDERER_H

#include <chrono>
#include <map>
#include <string>
#include <filesystem>
#include <iostream>

#include <boost/container/flat_map.hpp>

#undef min
#undef max

#include <plf_hive.h>

#include <imgui.h>
#include <imgui_impl_sdlrenderer2.h>

#include "Camera.h"
#include "Sprite.h"
#include "GlobalMacros.h"

namespace nv {
	class Renderer {
	protected:
		SDL_Renderer* m_renderer;
		
		Background* m_background = nullptr;

		template<typename T>
		using Layers = FlatOrderedMap<int, plf::hive<T*>>;

		Layers<Sprite> m_objects;
	public:
		Renderer(SDL_Renderer* renderer); 
		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&)      = delete;

		inline SDL_Renderer* get() {
			return m_renderer;
		}

		void move(int dx, int dy) noexcept;

		void clear() noexcept;

		void setBackground(Background* background) noexcept;
		void addObj(Sprite* obj, int layer);
		void removeObj(const ID& ID, int layer);

		void render() noexcept; 
	};
}

#endif