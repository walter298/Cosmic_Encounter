#include "Scene.h"

#include <print>
#include <thread> //sleep for framerate

#include "data_util/BasicJsonSerialization.h"

void nv::Scene::render() {
	m_objectLayers.forEach([this](int layer, const auto& obj) {
		using Type = std::remove_cvref_t<decltype(obj)>;
		if constexpr (IsClassTemplate<std::unique_ptr, Type>::value || std::is_pointer_v<Type>) {
			obj->render(renderer);
		} else {
			unrefwrap(obj).render(renderer);
		}
		return STAY_IN_LOOP;
	});
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
	runEvents(std::get<EventData<const Uint8*>>(m_eventData), m_keystate);

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
	struct IsReferenceLayer : public std::disjunction<
		nv::IsClassTemplate<std::reference_wrapper, typename Range::value_type>,
		std::is_pointer<typename Range::value_type>
	> {};
}

nv::Scene::Scene(std::string_view absFilePath, SDL_Renderer* renderer, TextureMap& texMap, FontMap& fontMap)
	: renderer{ renderer }, texMap{texMap}, fontMap{ fontMap }
{
	std::ifstream sceneFile{ absFilePath.data() };
	assert(sceneFile.is_open());

	auto rootJson = json::parse(sceneFile);

	auto loadRenderObjectLayer = [&, this](const json& objsJson, auto& objs, auto&... args) {
		objs.reserve(objsJson.size());
		for (const auto& objJson : objsJson) {
			objs.emplace(renderer, objJson, args...);
		}
	};

	auto loadObjectLayers = [&, this](const json& objsJson, int layer) {
		auto& sprites  = std::get<0>(m_objectLayers.layers[layer]);
		auto& textures = std::get<1>(m_objectLayers.layers[layer]);
		auto& text     = std::get<2>(m_objectLayers.layers[layer]);
		auto& rects    = std::get<3>(m_objectLayers.layers[layer]);

		loadRenderObjectLayer(objsJson.at(typeid(Sprite).name()), sprites, texMap);
		loadRenderObjectLayer(objsJson.at(typeid(Texture).name()), textures, texMap);
		loadRenderObjectLayer(objsJson.at(typeid(Text).name()), text, fontMap);
		rects = objsJson.at(objsJson.at(typeid(Text).name())).get<plf::hive<Rect>>();
	};

	//load in render objects
	auto& objectLayersJson = rootJson["objects"];
	for (auto& jsonLayer : objectLayersJson) {
		int layer = jsonLayer["layer"].get<int>();
		loadObjectLayers(jsonLayer, layer);
	}

	//load in special points
	auto& pointsJson = rootJson["special_points"];
	m_specialPoints.reserve(pointsJson.size());
	for (const auto& pointJson : pointsJson) {
		m_specialPoints.emplace(pointJson["name"].get<std::string>(), pointJson["point"].get<SDL_Point>());
	}
	sceneFile.close();
}

SDL_Point nv::Scene::getSpecialPoint(std::string_view name) const noexcept {
	static std::string key;
	key = name;
	return m_specialPoints.at(key);
}

nv::ID<nv::TextInput> nv::Scene::addTextInput(nv::TextInput&& textInput) {
	ID<TextInput> id;
	m_textInputs.emplace_back(std::move(textInput), id);
	return id;
}

void nv::Scene::operator()() {
	running = true;

	const auto waitTime = 1000ms / FPS;

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
	auto overlayImpl = [&, this](auto& thisObjectLayers, auto& otherObjectLayer) {
		if (otherObjectLayer.empty()) {
			return;
		}
		using Type = typename std::remove_cvref_t<decltype(otherObjectLayer)>::value_type;

		if constexpr (IsClassTemplate<std::unique_ptr, Type>::value) {
			auto ptrView = ranges::transform_view(otherObjectLayer, [](auto& obj) {
				return obj.get();
			});
			using Ptr = std::tuple_element_t<0, typename GetTemplateTypes<Type>::Types>;
			auto& objPtrs = std::get<plf::hive<Ptr*>>(thisObjectLayers);
			objPtrs.insert(ptrView.begin(), ptrView.end());
		} else if constexpr (std::is_pointer_v<Type> || IsClassTemplate<std::reference_wrapper, Type>::value) {
			auto& objs = std::get<plf::hive<Type>>(thisObjectLayers);
			objs.insert(otherObjectLayer.begin(), otherObjectLayer.end());
		} else {
			auto refView = ranges::transform_view(otherObjectLayer, [](auto& obj) {
				return std::ref(obj);
			});
			auto& objRefs = std::get<plf::hive<std::reference_wrapper<Type>>>(thisObjectLayers);
			objRefs.insert(refView.begin(), refView.end());
		}
	};

	for (auto& [layer, otherObjectLayers] : scene.m_objectLayers.layers) {
		auto& thisObjectLayers = m_objectLayers.layers[layer];
		auto& currLayer = m_objectLayers.layers[layer];
		forEachDataMember([&](auto& otherObjectLayer) {
			overlayImpl(currLayer, otherObjectLayer);
			return STAY_IN_LOOP;
		}, otherObjectLayers);
	}
}

void nv::Scene::deoverlay() {
	for (auto& [layer, objLayer] : m_objectLayers.layers) {
		auto objRefs = filterDataMembers<IsReferenceLayer>(m_objectLayers.layers[layer]);
		forEachDataMember([](auto& objs) {
			objs.clear();
			return STAY_IN_LOOP;
		}, objRefs);
	}
}

void nv::Scene::printElements() const {
	for (const auto& [layer, objects] : m_objectLayers.layers) {
		std::println("Layer {}", layer);
		forEachDataMember([](auto& objs) {
			std::print("[");
			for (const auto& obj : objs) {
				using Type = std::remove_cvref_t<decltype(obj)>;
				if constexpr (IsClassTemplate<std::unique_ptr, Type>::value || std::is_pointer_v<Type>) {
					std::print("{} ", obj->getName());
				} else {
					std::print("{} ", unrefwrap(obj).name);
				} 
			}
			std::println("]");
			return STAY_IN_LOOP;
		}, objects);
	}
}
