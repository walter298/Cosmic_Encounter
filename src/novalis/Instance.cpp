#include "Instance.h"

void nv::Instance::quit() {
	m_spriteMap.clear();
	m_textMap.clear();
	m_backgroundMap.clear();

	Text::closeFonts();

	TTF_Quit();
	IMG_Quit();
	Mix_Quit();

	SDL_DestroyRenderer(m_SDLRenderer);
	SDL_DestroyWindow(m_SDLWindow);
	SDL_Quit();
}

nv::Instance::Instance(std::string windowTitle)
	: m_SDLWindow{ SDL_CreateWindow(windowTitle.c_str(), 0, 0, NV_SCREEN_WIDTH, NV_SCREEN_HEIGHT, SDL_WINDOW_OPENGL) },
	m_SDLRenderer{ SDL_CreateRenderer(m_SDLWindow, -1, SDL_RENDERER_ACCELERATED) }
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0 ||
		Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0 ||
		TTF_Init() != 0 || IMG_Init(IMG_INIT_JPG & IMG_INIT_PNG) != 0) 
	{
		quit();
		throw std::runtime_error("Failed to initialize SDL");
	}
	nv::workingDirectory(); //set working directory
	Text::loadFonts();
}

nv::Instance::~Instance() {
	quit();
}

SDL_Window* nv::Instance::getRawWindow() noexcept {
	return m_SDLWindow;
}

SDL_Renderer* nv::Instance::getRawRenderer() noexcept {
	return m_SDLRenderer;
}

nv::Sprite nv::Instance::getSprite(std::string name) const {
	return m_spriteMap.at(name);
}

nv::Background nv::Instance::getBackground(std::string name) const {
	return m_backgroundMap.at(name);
}

nv::Text nv::Instance::getText(std::string name) const {
	return m_textMap.at(name);
}

void nv::Instance::loadObjsFromDir(std::string absDirPath) {
	using namespace std::filesystem;
	if (!exists(absDirPath)) {
		throw std::runtime_error("Error: " + absDirPath + " does not exist.\n");
	}

	std::vector<std::string> subDirectoryPaths; //file paths of directories within the current directory

	for (const auto& entry : directory_iterator(absDirPath)) {
		auto currentPath = entry.path().string();
		std::replace(currentPath.begin(), currentPath.end(), '\\', '/');

		if (!entry.is_directory()) {
			auto fileExt = fileExtension(currentPath);
			if (fileExt.has_value()) {
				auto& ext = fileExt.value();
				if (ext == "nv_sprite") {
					Sprite sprite{ m_SDLRenderer, currentPath };
					m_spriteMap[sprite.getName()] = std::move(sprite);
				} else if (ext == "nv_text") {
					Text text{ m_SDLRenderer, currentPath };
					m_textMap[text.getName()] = std::move(text);
				} else if (ext == "nv_background") {
					Background background{ m_SDLRenderer, currentPath };
					m_backgroundMap[background.getName()] = std::move(background);
				}
			}
		} else {
			subDirectoryPaths.push_back(std::move(currentPath));
		}
	}

	for (const auto& nestedPath : subDirectoryPaths) {
		loadObjsFromDir(nestedPath);
	}
}