#pragma once

#include "Event.h"
#include "Text.h"

namespace nv {
	class EventHandler {
	private:
		SDL_Event m_SDLEvt;
		
		MouseData m_mouseData;

		std::string m_textInputBuff;

		Keymap m_keymap;
		const Uint8* m_keystate = SDL_GetKeyboardState(nullptr);

		template<typename EventType>
		using Events = std::vector<std::pair<EventType, ID>>;

		Events<Event<>> m_periodicEvents;
		Events<MouseEvent> m_mouseEvents;
		Events<KeyboardEvent> m_keyboardEvents;
		Events<Event<>> m_quitEvents;

		Events<TextInput> m_textInputs;
		TextInput* m_currEditedTextInput = nullptr;
		void selectTextInput();

		template<typename EventData>
		void removeEventImpl(Events<EventData>& events, ID id) {
			auto evtIt = binaryFind(events, id, &std::pair<EventData, ID>::second);
			assert(evtIt != events.end());
			events.erase(evtIt);
		}

		template<typename Events, typename Func, typename... Args>
		void pushCancellableEvent(Events& events, Func&& func, ID id, const Args&... args) {
			events.emplace_back([this, events = std::ref(events), func = std::forward<Func>(func), id = id](const Args&... args) mutable {
				if (func(args...)) {
					removeEventImpl(events.get(), id);
				}
			}, id);
		}

		template<typename Func, typename Events>
		ID addEventImpl(Func&& func, Events& events) {
			ID id;

			if constexpr (std::same_as<ResultOfNonOverloaded<Func>, bool>) {
				typename FunctionTraits<std::decay_t<Func>>::args funcArgs;
				std::apply([&, this](const auto&... args) { pushCancellableEvent(events, std::forward<Func>(func), id, args...); }, funcArgs);
			} else {
				events.emplace_back(std::forward<Func>(func), id);
			}
			return id;
		}
	public:
		template<std::invocable Func>
		ID addPeriodicEvent(Func&& evt) {
			return addEventImpl(std::forward<Func>(evt), m_periodicEvents);
		}
		void removePeriodicEvent(ID Id);

		template<std::invocable<MouseData> Func>
		ID addMouseEvent(Func&& evt) {
			return addEventImpl(std::forward<Func>(evt), m_mouseEvents);
		}
		template<std::invocable<const Keymap&> Func>
		ID addKeyboardEvent(Func&& evt) {
			return addEventImpl(std::forward<Func>(evt), m_keyboardEvents);
		}
		template<std::invocable Func>
		ID addQuitEvent(Func&& evt) {
			return addEventImpl(std::forward<Func>(evt), m_quitEvents);
		}
		ID addTextInput(TextInput&& textInput);

		void operator()() noexcept;
	};
}
