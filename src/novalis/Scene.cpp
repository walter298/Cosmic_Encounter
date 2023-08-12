#include "Scene.h"

void nv::Scene::setObjectPositions() {
	for (const auto& positionSetter : m_objectPositionSetters) {
		positionSetter();
	}
}

nv::Scene::Scene(std::string absPath, NovalisInstance& instance) : m_renderer(instance.renderer()) {
	FileData sceneData{ absPath };

	using namespace std::literals;

	auto backgroundData = sceneData.getDataSection("BACKGROUNDS {"s);

	//end program if no backgrounds/background image paths were specified 
	if (!backgroundData.has_value()) {
		throw std::runtime_error("Error: no background data specified in " + absPath);
	}
	if (backgroundData.value()->data().size() == 0) { //optional is never empty
		throw std::runtime_error("Error: no background image paths specified in " + absPath);
	}

	//add backgrounds to scene
	const auto& backgrounds = backgroundData.value()->data();

	for (const auto& backgroundDatum : backgrounds) {
		addObj(std::make_unique<Background>(instance.renderer().get(), backgroundDatum));
	}

	auto objData = sceneData.getMultipleDataSections("OBJECT {"s);
	if (objData.has_value()) {
		const auto& objectsData = objData.value();
		for (const auto& nestedObjData : objectsData) {
			auto name = nestedObjData->getNestedData("NAME {");

			if (!name.has_value()) {
				throw std::runtime_error("Error: no name data for object was specified in " + absPath);
			} else if (name.value()->data().size() != 1) {
				throw std::runtime_error("Error: incorrect name data format for object in " + absPath);
			}

			const auto& nameVal = name.value()->data().front(); //optional is never empty
			auto newObj = instance.getObj<Sprite>(nameVal);
			addObj(newObj);
			if (nestedObjData->getNestedData("POSITION {").has_value()) {
				auto coordData = nestedObjData->getNestedData("POSITION {");
				const auto& coords = coordData.value()->data().front();
				int rx, ry, wx, wy;
				parseUnderscoredNums(coords, rx, ry, wx, wy);
				m_objectPositionSetters.push_back(
					[movedObj = std::move(newObj), rx, ry, wx, wy] {
						movedObj->setRenPos(rx, ry);
						movedObj->setWorldPos(wx, wy);
					}
				);
			}
		}
	}
}

void nv::Scene::addObj(RenderObjPtr obj) {
	m_objects.push_back(obj.get());
	m_owningObjects.push_back(std::move(obj)); 
}

void nv::Scene::addObj(RenderObj* obj) {
	m_objects.push_back(obj);
}

void nv::Scene::addEvent(Event evt) {
	m_events.push_back(std::move(evt));
}

void nv::Scene::cancelEvent(int ID) {
	auto evtPos = std::ranges::find_if(m_events,
		[&ID](const auto& evt) { return evt.getID() == ID; }
	);
	if (evtPos != m_events.end()) {
		m_events.erase(evtPos);
	}
}

void nv::Scene::addButton(Button* btn) {
	m_buttons.push_back(btn);
}

void nv::Scene::removeButton(int ID) {
	auto btnPos = std::ranges::find_if(m_buttons,
		[&ID](const auto& evt) { return evt->getID() == ID; }
	);
	if (btnPos != m_buttons.end()) {
		m_buttons.erase(btnPos);
	}
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
	setObjectPositions();

	for (const auto& obj : m_objects) {
		m_renderer.addObj(obj);
	}

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
		for (const auto& btn : m_buttons) {
			(*btn)();
		}
		
		const auto now = system_clock::now();
		if (now < endTime) {
			std::this_thread::sleep_for(endTime - now);
		}
		m_renderer.render();
	}
}

