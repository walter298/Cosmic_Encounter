#include "Scene.h"

nv::Scene::Scene(std::string absFilePath, Instance& instance) : m_renderer(instance.getRawRenderer()) {
	std::ifstream sceneFile{ absFilePath };
	assert(sceneFile.is_open());

	auto jsonData = json::parse(sceneFile);

	m_background = instance.getBackground(jsonData["background"].get<std::string>());

	auto objectNames = jsonData["sprites"].get<std::vector<std::string>>();
	for (const auto& name : objectNames) {
		auto sprite = instance.getSprite(name);

		auto ren = jsonData["sprites"][name]["ren"].get<Rect>();
		sprite.setRenPos(ren.rect.x, ren.rect.y);
		sprite.setRenSize(ren.rect.w, ren.rect.h);

		auto world = jsonData["sprites"][name]["world"].get<Rect>();
		sprite.setWorldPos(world.rect.x, world.rect.y);
		sprite.setWorldSize(world.rect.w, world.rect.h);
	}
	sceneFile.close();
}

void nv::Scene::addObj(Sprite&& obj, int layer) noexcept {
	m_sprites.push_back(std::move(obj));
	m_renderer.addObj(&m_sprites.back(), layer);
}

void nv::Scene::addObj(Text&& obj, int layer) noexcept {
	m_texts.push_back(std::move(obj));
	m_renderer.addObj(&m_sprites.back(), layer);
}

void nv::Scene::render(RenderObj* obj, int layer) noexcept {
	m_renderer.addObj(obj, layer);
}

void nv::Scene::stopRendering(ID<RenderObj> ID) noexcept {
	m_renderer.removeObj(ID);
}

void nv::Scene::addEvent(Event&& evt) noexcept {
	m_events.push_back(std::move(evt));
}

void nv::Scene::addEvent(Event* evt) noexcept {
	m_nonOwningEvents.push_back(evt);
}

void nv::Scene::cancelEvent(ID<Event> ID) noexcept {
	removeIfHasID(m_nonOwningEvents, ID);
}

void nv::Scene::addButton(Button&& btn) noexcept {
	m_buttons.push_back(std::move(btn));
}

void nv::Scene::addButton(Button* btn) noexcept {
	m_nonOwningButtons.push_back(btn);
}

void nv::Scene::removeButton(ID<Button> ID) noexcept {
	removeIfHasID(m_nonOwningButtons, ID);
}

void nv::Scene::endScene(Scene::EndReason endReason) noexcept {
	m_renderer.clear();
	m_endReason = endReason;
	m_running = false;
}

nv::Scene::EndReason nv::Scene::endReason() const noexcept {
	return m_endReason;
}

void nv::Scene::camMove(int dx, int dy) {
	//todo
}

void nv::Scene::execute() {
	m_running = true;

	Event quitEvt{ [this] { m_running = false; },
		[] { return InputHandler::getInstance().eventStates(SDL_QUIT); } };

	addEvent(std::move(quitEvt));

	constexpr auto waitTime = 1000ms / NV_FPS;

	while (m_running) {
		auto endTime = system_clock::now() + waitTime;

		InputHandler::getInstance().run(); //update inputs 
		
		for (auto& evt : m_events) {
			evt();
		}
		for (auto& evt : m_nonOwningEvents) {
			(*evt)();
		}
		for (auto& btn : m_buttons) {
			btn();
		}
		for (auto& btn : m_nonOwningButtons) {
			(*btn)();
		}
		
		const auto now = system_clock::now();
		if (now < endTime) {
			std::this_thread::sleep_for(endTime - now);
		}
		m_renderer.render();
	}
}

