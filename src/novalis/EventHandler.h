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
	public:
		ID addPeriodicEvent(Event<>&& evt);
		ID addMouseEvent(MouseEvent&& evt);
		ID addKeyboardEvent(KeyboardEvent&& evt);
		ID addQuitEvent(Event<>&& evt);
		ID addTextInput(TextInput&& textInput);

		void operator()() noexcept;
	};
}
