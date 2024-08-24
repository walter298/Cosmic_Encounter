#ifndef SCENE_H
#define SCENE_H

#include <print>
#include <tuple>

#include <plf_hive.h>

#include <nlohmann/json.hpp>

#include "data_util/Algorithms.h"
#include "data_util/DataStructures.h"
#include "data_util/Reflection.h"

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

		template<typename Object>
		using IDObjects = std::vector<std::pair<Object, ID<Object>>>;

		template<typename Ret, typename... EventParams>
		using Events = IDObjects<Event<Ret, EventParams...>>;

		template<typename... EventArgs>
		struct EventData {
			Events<void, EventArgs...> events;
			Events<bool, EventArgs...> cancellableEvents;

			std::vector<typename Events<bool, EventArgs...>::iterator> cancelledEventIterators;
		};

		Keymap m_keyMap;
		const Uint8* m_keystate = SDL_GetKeyboardState(nullptr);

		std::tuple<EventData<>, EventData<MouseData>, EventData<const Keymap&>> m_eventData;

		IDObjects<TextInput> m_textInputs;
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
		auto find(this auto&& self, int layer, std::string_view name)
			requires(RenderObject<std::unwrap_reference_t<Object>&>) 
		{
			decltype(auto) objs = std::get<plf::hive<Object>>(self.m_objectLayers.at(layer));
			auto objIt = ranges::find_if(objs, [&](const auto& obj) { 
				return unrefwrap(obj).getName() == name;
			});
			if (objIt == objs.end()) {
				std::println("Error: could not find {} at layer {}", name, layer);
				self.printElements();
				exit(-5555);
			}
			
			return StableRef{ objs, objIt };
		}

		template<typename Object>
		auto addObject(Object&& object, int layer) 
			requires(RenderObject<std::unwrap_reference_t<Object&>>) 
		{
			decltype(auto) objects = std::get<plf::hive<std::remove_cvref_t<Object>>>(m_objectLayers[layer]);
			return StableRef{ objects, objects.insert(std::forward<Object>(object)) };
		}
	private:
		template<typename Object, typename Objects, typename Transform>
		void eraseImpl(ID<Object> id, Objects& objects, Transform transform) {
			auto objIt = binaryFind(objects, id, transform);
			assert(objIt != objects.end());
			objects.erase(objIt);
		}
	public:
		template<typename EventType>
		void removeEvent(ID<EventType> id) {
			using EventTypeTPs     = typename GetTemplateTypes<EventType>::Types; //will be of "one" type like void(int, int)
			using EventInfo        = FunctionTraits<std::tuple_element_t<0, EventTypeTPs>>;
			using EventArgs        = typename EventInfo::Args;
			using EventRet         = typename EventInfo::Ret;
			using MatchedEventData = typename GetParameterizedTypeFromTuple<EventData, EventArgs>::type;

			auto& eventData = std::get<MatchedEventData>(m_eventData);
			auto getID = &std::pair<EventType, ID<EventType>>::second;
			if constexpr (std::same_as<EventRet, bool>) {
				eraseImpl(id, eventData.cancellableEvents, getID);
			} else {
				eraseImpl(id, eventData.events, getID);
			}
		}
	
		template<typename Func>
		auto addEvent(Func&& func) {
			using FuncInfo = FunctionTraits<std::decay_t<Func>>;
			using FuncArgs = typename FuncInfo::Args;
			using FuncRet  = typename FuncInfo::Ret;
			using FuncSig  = typename FuncInfo::Sig;

			using IDSpecialization = ID<typename GetParameterizedTypeFromTuple<Event, FuncSig>::type>;
			IDSpecialization id;

			using EventDataSpecialization = typename GetParameterizedTypeFromTuple<EventData, FuncArgs>::type;
			auto& eventData = std::get<EventDataSpecialization>(m_eventData);

			if constexpr (std::same_as<FuncRet, bool>) {
				eventData.cancellableEvents.emplace_back(std::forward<Func>(func), id);
			} else {
				eventData.events.emplace_back(std::forward<Func>(func), id);
			}
			return id;
		}
		ID<TextInput> addTextInput(nv::TextInput&& textInput);

		void operator()();

		void overlay(Scene& scene);
		void deoverlay();

		void printElements() const;
	};
}

#endif