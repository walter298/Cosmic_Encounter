#include "Event.h"

bool nv::InputHandler::mouseHeld(int mask) {
	int x = 0, y = 0;
	return (SDL_GetMouseState(&x, &y) & mask);
}

void nv::InputHandler::updateMouseStates() {
	m_lMX = m_mouseX, m_lMY = m_mouseY;
	SDL_GetMouseState(&m_mouseX, &m_mouseY); //update mouse coordinates

	m_lastMouseClickState = m_mouseClicked;
	m_mouseClicked        = m_inputStates[SDL_MOUSEBUTTONDOWN];

	/*update whether we just clicked with the left and right
	mouse buttons and whether we are holding them down*/
	m_lastLeftMouseButtonClickState  = m_leftMouseButtonClickState;
	m_leftMouseButtonClickState      = mouseHeld(SDL_BUTTON_LMASK);
	m_lastRightMouseButtonClickState = m_rightMouseButtonClickState;
	m_rightMouseButtonClickState     = mouseHeld(SDL_BUTTON_RMASK);
}

nv::InputHandler& nv::InputHandler::getInstance() {
	static InputHandler handler;
	return handler;
}

std::pair<int, int> nv::InputHandler::mouseChange() {
	return std::make_pair(m_mouseX - m_lMX, m_mouseY - m_lMY);
}

std::pair<int, int> nv::InputHandler::mouse() {
	return std::make_pair(m_mouseX, m_mouseY);
}

bool nv::InputHandler::leftMouseClicked() {
	return !m_lastLeftMouseButtonClickState && m_leftMouseButtonClickState;
}

bool nv::InputHandler::leftMouseHeld() {
	return m_lastLeftMouseButtonClickState && m_leftMouseButtonClickState;
}

bool nv::InputHandler::leftMouseReleased() {
	return m_lastLeftMouseButtonClickState && !m_leftMouseButtonClickState;
}

bool nv::InputHandler::rightMouseClicked() {
	return !m_lastRightMouseButtonClickState && m_rightMouseButtonClickState;
}

bool nv::InputHandler::rightMouseHeld() {
	return m_lastRightMouseButtonClickState && m_rightMouseButtonClickState;
}

bool nv::InputHandler::rightMouseReleased() {
	return m_lastRightMouseButtonClickState && !m_rightMouseButtonClickState;
}

void nv::InputHandler::run() {
	for (auto& [input, state] : m_inputStates) {
		state = false;
	}
	while (SDL_PollEvent(&m_evt)) {
		m_inputStates[static_cast<SDL_EventType>(m_evt.type)] = true;
		if (m_usingImGui) {
			ImGui_ImplSDL2_ProcessEvent(&m_evt);
		}
	}
	
	updateMouseStates();
}

void nv::InputHandler::useImGui() {
	m_usingImGui = true;
}

nv::ID<nv::Event> nv::Event::getID() const noexcept {
	return m_ID;
}

void nv::Event::operator()() {
	m_func();
}