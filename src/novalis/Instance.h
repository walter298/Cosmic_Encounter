#ifndef INSTANCE_H
#define INSTANCE_H

#include <string_view>

#include <SDL2/SDL.h>

namespace nv {
	class Instance {
	private:
		void quit();
	public:
		SDL_Window* window     = nullptr;
		SDL_Renderer* renderer = nullptr;

		Instance(std::string_view windowTitle);
		Instance(Instance&&) = delete;
		~Instance() noexcept;
	};
}

#endif