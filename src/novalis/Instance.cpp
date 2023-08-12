#include "Instance.h"

void nv::NovalisInstance::quit() {
	m_objMap.clear();
	m_renderer.clear();

	Text::closeFonts();
	TTF_Quit();

	IMG_Quit();
	Mix_Quit();

	SDL_DestroyRenderer(m_SDLRenderer);
	SDL_DestroyWindow(m_window);
	SDL_Quit();
}

nv::NovalisInstance::NovalisInstance(std::string windowTitle) 
	: m_window{ m_window = SDL_CreateWindow(windowTitle.c_str(), 0, 0, NV_SCREEN_WIDTH, NV_SCREEN_HEIGHT, SDL_WINDOW_OPENGL) }, 
	m_SDLRenderer{ SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED) }, 
	m_renderer{ m_SDLRenderer }
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

nv::NovalisInstance::~NovalisInstance() {
	quit();
}

void nv::NovalisInstance::loadObj(std::string absFilePath) {
	using namespace std::filesystem;
	if (!exists(absFilePath)) {
		throw std::runtime_error("Error: " + absFilePath + " does not exist.\n");
	}
	auto fileExtensionRes = fileExtension(absFilePath);

	RenderObjPtr obj;

	if (fileExtensionRes.value() == ".nv_sprite") {
		obj = std::make_unique<Sprite>(m_renderer.get(), absFilePath);
	} 
	else if (fileExtensionRes.value() == ".nv_txt") {
		obj = std::make_unique<Text>(m_renderer.get(), absFilePath);
	}
	else {
		throw std::runtime_error("Error: " + absFilePath + " does not have a valid file extension\n");
	}
	m_objMap.emplace(obj->getName(), std::move(obj));
}

void nv::NovalisInstance::loadObjsFromDir(std::string absDirPath) {
	using namespace std::filesystem;
	if (!exists(absDirPath)) {
		throw std::runtime_error("Error: " + absDirPath + " does not exist.\n");
	}

	std::vector<std::string> subDirectoryPaths; //file paths of directories within the current directory

	for (const auto& entry : directory_iterator(absDirPath)) {
		auto currentPath = entry.path().string();
		std::replace(currentPath.begin(), currentPath.end(), '\\', '/');

		if (!entry.is_directory()) {
			loadObj(currentPath);
		} else {
			subDirectoryPaths.push_back(std::move(currentPath));
		}
	}

	for (const auto& nestedPath : subDirectoryPaths) {
		loadObjsFromDir(nestedPath);
	}
}

void nv::NovalisInstance::removeObj(std::string name) {
	m_objMap.erase(name);
}