#ifndef INSTANCE_H
#define INSTANCE_H

#include <string_view>

#include <SDL2/SDL.h>

namespace nv {
	class Instance {
	private:
		void quit();

		int m_screenWidth  = 0;
		int m_screenHeight = 0;
	public:
		SDL_Window* window     = nullptr;
		SDL_Renderer* renderer = nullptr;

		Instance(std::string_view windowTitle) noexcept;
		Instance(Instance&&) = delete;
		~Instance() noexcept;

		int getScreenWidth() const noexcept;
		int getScreenHeight() const noexcept;
	};
}

#endif