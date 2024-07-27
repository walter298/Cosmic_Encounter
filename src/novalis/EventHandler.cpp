#include "EventHandler.h"

void nv::EventHandler::selectTextInput() {
	auto editedTexIt = ranges::find_if(m_textInputs, [this](const auto& textInput) {
		return textInput.first.getRect().containsCoord(m_mouseData.x, m_mouseData.y);
	});
	if (editedTexIt == m_textInputs.end()) {
		m_currEditedTextInput = nullptr;
	} else {
		m_currEditedTextInput = &(editedTexIt->first);
	}
}

void nv::EventHandler::removePeriodicEvent(ID id) {
	removeEventImpl(m_periodicEvents, id);
}

nv::ID nv::EventHandler::addTextInput(TextInput&& textInput) {
	ID id;
	m_textInputs.emplace_back(std::move(textInput), id);
	return id;
}

void nv::EventHandler::operator()() noexcept {
	auto runEvents = [](auto& events, const auto&... inputs) {
		for (auto& [evt, ID] : events) {
			evt(inputs...);
		}
	};

	runEvents(m_periodicEvents);

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
			runEvents(m_quitEvents);
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
	runEvents(m_mouseEvents, m_mouseData);
	
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