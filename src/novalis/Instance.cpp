#include "Instance.h"

#include "DataUtil.h"

#include <print>

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

void nv::Instance::quit() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	IMG_Quit();
	Mix_Quit();
	TTF_Quit();
	SDL_Quit();
}

nv::Instance::Instance(std::string_view windowTitle) noexcept {
	auto exitWithError = [this] {
		std::println("{}", SDL_GetError());
		quit();
		exit(-1);
	};

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0 || //returns zero on sucess
		Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0 ||
		TTF_Init() != 0 || 
		IMG_Init(IMG_INIT_PNG) == 0)
	{
		exitWithError();
	}

	//get screen width and height
	SDL_DisplayMode dm;
	SDL_GetCurrentDisplayMode(0, &dm);
	m_screenWidth = dm.w;
	m_screenHeight = dm.h;

	window = SDL_CreateWindow(windowTitle.data(), 0, 0, m_screenWidth, m_screenHeight, SDL_WINDOW_OPENGL);
	if (window == nullptr) {
		exitWithError();
	}
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == nullptr) {
		exitWithError();
	}

	constexpr int WIDTH_ANCHOR = 2560;
	constexpr int HEIGHT_ANCHOR = 1440;
	SDL_RenderSetLogicalSize(renderer, WIDTH_ANCHOR, HEIGHT_ANCHOR);

	workingDirectory(); //initialize local static inside workingDirectory
}

nv::Instance::~Instance() noexcept {
	quit();
}

int nv::Instance::getScreenWidth() const noexcept {
	return m_screenWidth;
}

int nv::Instance::getScreenHeight() const noexcept {
	return m_screenHeight;
}
