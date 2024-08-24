#include "Scene.h"

#include <thread> //sleep for framerate

void nv::Scene::render() {
	for (const auto& [layer, objLayer] : m_objectLayers) {
		forEachDataMember([](const auto& objs) {
			for (const auto& obj : objs) {
				unrefwrap(obj).render();
			}
			return STAY_IN_LOOP;
		}, objLayer);
	}
}

void nv::Scene::selectTextInput() {
	auto editedTexIt = ranges::find_if(m_textInputs, [this](const auto& textInput) {
		return textInput.first.getRect().containsCoord(m_mouseData.x, m_mouseData.y);
	});
	if (editedTexIt == m_textInputs.end()) {
		m_currEditedTextInput = nullptr;
	} else {
		m_currEditedTextInput = &(editedTexIt->first);
	}
}

void nv::Scene::executeEvents() {
	auto runEvents = [](auto& eventData, const auto&... inputs) {
		eventData.events;
		for (auto& [evt, ID] : eventData.events) {
			evt(inputs...);
		}
		for (auto it = eventData.cancellableEvents.begin(); it != eventData.cancellableEvents.end(); it++) {
			auto& evt = it->first;
			if (evt(inputs...)) {
				eventData.cancelledEventIterators.push_back(it);
			}
		}
		if (!eventData.cancelledEventIterators.empty()) {
			eraseMultipleIterators(eventData.cancellableEvents, eventData.cancelledEventIterators);
		}
	};
	
	runEvents(std::get<EventData<>>(m_eventData));

	int deltaX = 0;
	int deltaY = 0;

	auto setMouseState = [this](Uint8 btn, MouseButtonState newState) {
		switch (btn) {
		case SDL_BUTTON_LEFT:
			m_mouseData.left = newState;
			break;
		case SDL_BUTTON_MIDDLE:
			m_mouseData.mid = newState;
			break;
		case SDL_BUTTON_RIGHT:
			m_mouseData.right = newState;
			break;
		}
	};

	bool textEditing = false;

	while (SDL_PollEvent(&m_SDLEvt)) {
		switch (m_SDLEvt.type) {
		case SDL_QUIT:
			running = false;
			break;
		case SDL_MOUSEBUTTONDOWN:
			setMouseState(m_SDLEvt.button.button, MouseButtonState::Down);
			break;
		case SDL_MOUSEBUTTONUP:
			setMouseState(m_SDLEvt.button.button, MouseButtonState::Released);
			selectTextInput();
			break;
		case SDL_MOUSEMOTION:
			m_mouseData.x = m_SDLEvt.button.x;
			m_mouseData.y = m_SDLEvt.button.y;
			deltaX = m_SDLEvt.motion.x;
			deltaY = m_SDLEvt.motion.y;
			break;
		case SDL_TEXTINPUT:
			m_textInputBuff = m_SDLEvt.text.text;
			textEditing = true;
			break;
		}
	}
	m_mouseData.deltaX = deltaX;
	m_mouseData.deltaY = deltaY;
	runEvents(std::get<EventData<MouseData>>(m_eventData), m_mouseData);

	//text editing
	if (m_currEditedTextInput == nullptr) {
		return;
	}
	if (m_keystate[SDL_SCANCODE_BACKSPACE]) {
		m_currEditedTextInput->pop();
	} else if (!m_textInputBuff.empty() && textEditing) {
		m_currEditedTextInput->append(m_textInputBuff);
	}
}

namespace {
	template<typename Range>
	struct ContainsReferenceWrappers : public nv::IsReferenceWrapper<typename Range::value_type> {};
}

nv::Scene::Scene(std::string_view absFilePath, SDL_Renderer* renderer, TextureMap& texMap, FontMap& fontMap)
	: renderer{ renderer }, texMap{texMap}, fontMap{ fontMap }
{
	std::ifstream sceneFile{ absFilePath.data() };
	assert(sceneFile.is_open());

	auto sceneJson = json::parse(sceneFile);

	auto loadObjectLayer = [&, this](const json& objsJson, auto& objs, auto&... args) {
		objs.reserve(objsJson.size());
		for (const auto& objJson : objsJson) {
			objs.emplace(renderer, objJson, args...);
		}
	};

	auto loadObjectLayers = [&, this](const json& objsJson, int layer) {
		auto& sprites  = std::get<0>(m_objectLayers[layer]);
		auto& textures = std::get<1>(m_objectLayers[layer]);
		auto& text     = std::get<2>(m_objectLayers[layer]);
		auto& rects    = std::get<3>(m_objectLayers[layer]);

		loadObjectLayer(objsJson.at(typeid(Sprite).name()), sprites, texMap);
		loadObjectLayer(objsJson.at(typeid(Texture).name()), textures, texMap);
		loadObjectLayer(objsJson.at(typeid(Text).name()), text, fontMap);
		loadObjectLayer(objsJson.at(typeid(Rect).name()), rects);
	};

	auto& objectLayersJson = sceneJson["objects"];
	for (auto& jsonLayer : objectLayersJson) {
		int layer = jsonLayer["layer"].get<int>();
		loadObjectLayers(jsonLayer, layer);
	}

	sceneFile.close();
}

nv::ID<nv::TextInput> nv::Scene::addTextInput(nv::TextInput&& textInput) {
	ID<TextInput> id;
	m_textInputs.emplace_back(std::move(textInput), id);
	return id;
}

void nv::Scene::operator()() {
	running = true;

	constexpr auto FPS = 180;
	constexpr auto waitTime = 1000ms / FPS;

	while (running) {
		auto endTime = std::chrono::system_clock::now() + waitTime;

		executeEvents();
		
		const auto now = std::chrono::system_clock::now();
		if (now < endTime) {
			std::this_thread::sleep_for(endTime - now);
		}
		SDL_RenderClear(renderer);
		render();
		SDL_RenderPresent(renderer);
	}
}

void nv::Scene::overlay(Scene& scene) {
	/*you can't overlay an already overlayed scene on another scene as 
	references don't get pushed*/
	
	auto overlayImpl = [](auto& thisObjectLayer, auto& otherObjectLayer) {
		forEachDataMember([](auto& thisObjects, auto& otherObjects) { 
			auto refView = ranges::transform_view(otherObjects, [](auto& obj) {
				return std::ref(obj);
			});
			
			thisObjects.insert(refView.begin(), refView.end());
			return STAY_IN_LOOP; 
		}, thisObjectLayer, otherObjectLayer);
	};

	for (auto& [layer, otherObjectLayer] : scene.m_objectLayers) {
		auto objRefs = filterDataMembers<ContainsReferenceWrappers>(m_objectLayers[layer]);
		overlayImpl(objRefs, otherObjectLayer);
	}
}

void nv::Scene::deoverlay() {
	for (auto& [layer, objLayer] : m_objectLayers) {
		auto objRefs = filterDataMembers<ContainsReferenceWrappers>(m_objectLayers[layer]);
		forEachDataMember([](auto& objs) {
			objs.clear();
			return STAY_IN_LOOP;
		}, objRefs);
	}
}

void nv::Scene::printElements() const {
	for (auto& [layer, objs] : m_objectLayers) {
		std::println("{}: ", layer);
		forEachDataMember([](const auto& objs) {
			for (const auto& obj : objs) {
				std::print("{} ", unrefwrap(obj).getName());
			}
			std::println("");
			return STAY_IN_LOOP;
		}, objs);
	}
}
