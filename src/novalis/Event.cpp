#include "Event.h"

nv::MouseMotionData::MouseMotionData(const SDL_MouseMotionEvent& mouseWheelEvt) noexcept
	: mouseX{ static_cast<int>(mouseWheelEvt.x) }, mouseY{ static_cast<int>(mouseWheelEvt.y) },
	  deltaX{ static_cast<int>(mouseWheelEvt.xrel) }, deltaY{ static_cast<int>(mouseWheelEvt.yrel) } {}

nv::MouseButtonData::MouseButtonData(const SDL_MouseButtonEvent& evt) noexcept
	: btn{
		[&evt]() {
			switch (evt.button) {
			case SDL_BUTTON_LEFT:
				return nv::MouseButton::Left;
				break;
			case SDL_BUTTON_RIGHT:
				return nv::MouseButton::Right;
				break;
			case SDL_BUTTON_MIDDLE:
				return nv::MouseButton::Middle;
				break;
			default:
				return nv::MouseButton::None;
				break;
			}
		}()
	}, state{
		[&evt]() {
			if (evt.type == SDL_MOUSEBUTTONDOWN) {
				return nv::MouseButtonState::Released;
			} else {
				return nv::MouseButtonState::Down;
			}
		}()
	}, x{ evt.x }, y{ evt.y }
{}

void nv::EventHandler::operator()() noexcept {
	auto runEvents = [](auto& events, const auto&... inputs) {
		for (auto& evt : events) {
			evt(inputs...);
		}
	};

	runEvents(m_periodicEvents);

	while (SDL_PollEvent(&m_SDLEvt)) {
		/*switch (m_SDLEvt.type) {
		case SDL_KEYDOWN || SDL_KEYUP:
			auto& keyEvt = m_SDLEvt.key;
			m_keymap[keyEvt.keysym.scancode] = static_cast<bool>(keyEvt.state);
			break;
		case SDL_MOUSEWHEEL:
			runEvents(m_mouseWheelEvents, static_cast<int>(m_SDLEvt.wheel.y));
			break;
		case SDL_MOUSEBUTTONDOWN:
			runEvents(m_mouseButtonEvents, m_SDLEvt.button);
			break;
		case SDL_MOUSEBUTTONUP:
			runEvents(m_mouseButtonEvents, m_SDLEvt.button);
			break;
		case SDL_MOUSEMOTION:
			runEvents(m_mouseMotionEvents, m_SDLEvt.motion);
			break;
		}*/
	}
	runEvents(m_keyboardEvents, m_keymap);
}