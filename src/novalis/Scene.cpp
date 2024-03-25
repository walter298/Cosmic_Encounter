#include "Scene.h"

nv::Scene::Scene(nv::Instance& instance) 
	: renderer{ instance.getRawRenderer() } {
}

nv::Scene::Scene(const std::string& absFilePath, Instance& instance) : renderer{ instance.getRawRenderer() } {
	std::ifstream sceneFile{ absFilePath };
	assert(sceneFile.is_open());

	auto jsonData = json::parse(sceneFile);

	//renderer.addObj(instance.getBackground(jsonData["background"].get<std::string>()), 0);

	auto objectNames = jsonData["sprites"].get<std::vector<std::string>>();
	for (const auto& name : objectNames) {
		auto sprite = instance.getSprite(name);

		auto ren = jsonData.at("sprites").at(name).at("layer").get<Rect>();
		sprite.ren.setPos(ren.rect.x, ren.rect.y);
		sprite.ren.setSize(ren.rect.w, ren.rect.h);

		auto world = jsonData.at("sprites")[name]["world"].get<Rect>();
		sprite.world.setPos(world.rect.x, world.rect.y);
		sprite.ren.setSize(world.rect.w, world.rect.h);

		auto layer = jsonData.at("sprites").at(name).at("layer").get<int>();
		renderer.addObj(&sprite, layer);
	}
	sceneFile.close();
}

nv::Sprite& nv::Scene::sprite(const std::string& name) {
	return *m_sprites.at(name).begin();
}

nv::Sprites& nv::Scene::spriteClones(const std::string& name) {
	return m_sprites.at(name);
}

void nv::Scene::endScene(Scene::EndReason endReason) noexcept {
	m_endReason = endReason;
	m_running = false;
}

nv::Scene::EndReason nv::Scene::endReason() const noexcept {
	return m_endReason;
}

void nv::Scene::execute() {
	m_running = true;

	eventHandler.addQuitEvent([this] { m_running = false; });

	constexpr auto waitTime = 1000ms / NV_FPS;

	while (m_running) {
		auto endTime = std::chrono::system_clock::now() + waitTime;

		eventHandler();
		
		const auto now = std::chrono::system_clock::now();
		if (now < endTime) {
			std::this_thread::sleep_for(endTime - now);
		}
		renderer.render();
	}
}