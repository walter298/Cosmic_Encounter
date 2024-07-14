#include "Scene.h"

#include <thread>

nv::Scene::Scene(std::string_view absFilePath, SDL_Renderer* renderer, TextureMap& texMap, FontMap& fontMap) 
	: renderer{ renderer }, texMap{texMap}, fontMap{ fontMap }
{
	std::ifstream sceneFile{ absFilePath.data() };
	assert(sceneFile.is_open());

	auto sceneJson = json::parse(sceneFile);

	auto loadObjects = [&, this](const json& objLayersRoot, std::invocable<int, const json&> auto forEachObj) {
		for (const auto& objLayer : objLayersRoot) {
			int layer = objLayer["layer"].get<int>();

			for (const auto& objJson : objLayer["objects"]) {
				forEachObj(layer, objJson);
			}
		}
	};

	//load sprites
	loadObjects(sceneJson["sprites"], [&, this](int layer, const json& objJson) {
		sprites[layer].emplace_back(renderer, objJson, texMap);
	});
	//load texture objects
	loadObjects(sceneJson["texture_objects"], [&, this](int layer, const json& objJson) {
		textures[layer].emplace_back(renderer, objJson, texMap);
	});
	//load text
	loadObjects(sceneJson["text"], [&, this](int layer, const json& objJson) {
		text[layer].emplace_back(renderer, objJson, fontMap);
	});
	//load rects
	loadObjects(sceneJson["rects"], [&, this](int layer, const json& objJson) {
		auto rect = objJson.get<Rect>();
		rect.renderer = renderer;
		rects[layer].push_back(std::move(rect));
	});
	
	sceneFile.close();
}

void nv::Scene::operator()() {
	running = true;

	eventHandler.addQuitEvent([this] { running = false; });

	constexpr auto FPS = 180;
	constexpr auto waitTime = 1000ms / FPS;

	while (running) {
		auto endTime = std::chrono::system_clock::now() + waitTime;

		eventHandler();
		
		const auto now = std::chrono::system_clock::now();
		if (now < endTime) {
			std::this_thread::sleep_for(endTime - now);
		}
		SDL_RenderClear(renderer);
		renderCopy(textures, sprites, rects, text);
		SDL_RenderPresent(renderer);
	}
}