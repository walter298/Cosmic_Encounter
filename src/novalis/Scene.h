#ifndef SCENE_H
#define SCENE_H

#include <print>
#include <tuple>

#include <nlohmann/json.hpp>

#include "Button.h"
#include "ID.h"
#include "Event.h"
#include "Sprite.h"
#include "Text.h"
#include "Texture.h"

namespace nv {
	class Scene {
	private:
		void render();

		ObjectLayers<Sprite, Texture, Text, Rect, SpriteRef, TextureRef, TextRef, RectRef> m_objectLayers;
		
		SDL_Event m_SDLEvt{};

		template<typename EventT>
		using Events = std::vector<std::pair<EventT, ID>>;

		Keymap m_keyMap;
		const Uint8* m_keystate = SDL_GetKeyboardState(nullptr);

		std::tuple<Events<Event<>>, Events<MouseEvent>, Events<KeyboardEvent>> m_callableEvents;

		Events<TextInput> m_textInputs;
		TextInput* m_currEditedTextInput = nullptr;
		std::string m_textInputBuff;
		void selectTextInput();
		
		MouseData m_mouseData;

		void executeEvents();
	public:
		SDL_Renderer* renderer;
		FontMap& fontMap;
		TextureMap& texMap;

		bool running = false;

		Scene(std::string_view path, SDL_Renderer* renderer, TextureMap& texMap, FontMap& fontMap);

		template<typename Object>
		auto& find(this auto&& self, int layer, std::string_view name) 
			requires(RenderObject<std::unwrap_reference_t<Object>&>) 
		{
			decltype(auto) objs = std::get<std::vector<Object>>(self.m_objectLayers.at(layer));
			auto objIt = ranges::find_if(objs, [&](const auto& obj) { 
				return unrefwrap(obj).getName() == name;
			});
			assert(objIt != objs.end());
		
			return *objIt;
		}

		template<typename Object>
		decltype(auto) addObject(Object&& object, int layer) 
			requires(RenderObject<std::unwrap_reference_t<Object&>>) 
		{
			decltype(auto) objects = std::get<std::vector<std::remove_cvref_t<Object>>>(m_objectLayers[layer]);
			objects.push_back(std::forward<Object>(object));
			return unrefwrap(objects.back());
		}

		template<typename EventType>
		void removeEvent(ID id) {
			auto& events = std::get<std::vector<EventType>>(m_callableEvents);
			auto evtIt = binaryFind(events, id, &EventType::second);
			assert(evtIt != events.end());
			events.erase(evtIt);
		}
	private:
		template<typename Events, typename Func>
		void pushCancellableEvent(Events& events, Func&& func, ID id) {
			events.emplace_back([this, events = std::ref(events), func = std::forward<Func>(func), id = id](const auto&... args) mutable {
				if (func(args...)) {
					removeEvent<ValueType<Events>>(id);
				}
			}, id);
		}

		template<typename T> 
		struct GetEventsType;

		template<typename... Ts> 
		struct GetEventsType<std::tuple<Ts...>> {
			using type = Events<Event<Ts...>>;
		};
	public:
		template<typename Func>
		ID addEvent(Func&& func) {
			ID id;

			using FuncArgs = typename FunctionTraits<std::decay_t<Func>>::args;
			using EventsType = typename GetEventsType<FuncArgs>::type;

			auto& events = std::get<EventsType>(m_callableEvents);

			if constexpr (std::same_as<ResultOfNonOverloaded<Func>, bool>) {
				pushCancellableEvent(events, std::forward<Func>(func), id);
			} else {
				events.emplace_back(std::forward<Func>(func), id);
			}
			return id;
		}
		ID addTextInput(nv::TextInput&& textInput);

		void operator()();

		void overlay(Scene& scene);
		void deoverlay();
	};
}

#endif