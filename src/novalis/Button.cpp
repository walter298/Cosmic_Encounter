#include "Button.h"

nv::Button::Button(const Rect& rect) noexcept
	: m_rect{ rect }
{
}

nv::ID nv::Button::getID() const noexcept {
	return m_ID;
}

void nv::Button::operator()(const MouseButtonData& mouseBtnData) {
	if (m_rect.isCoordContained(mouseBtnData.x, mouseBtnData.y)) {
		m_previouslyHovered = true;
	} else if (m_previouslyHovered) {
		m_previouslyHovered = false;
		m_previouslyUnhovered = true;
	}
	if (mouseBtnData.state == MouseButtonState::Down) {
		switch (mouseBtnData.btn) {
		case MouseButton::Left:
			m_previouslyLeftHeld = true;
			break;
		case MouseButton::Right:
			m_previouslyRightHeld = true;
			break;
		case MouseButton::Middle:
			m_previouslyMiddleHeld = true;
			break;
		}
	} else if (mouseBtnData.state == MouseButtonState::Released) {
		switch (mouseBtnData.btn) {
		case MouseButton::Left:
			m_leftReleased = true;
			break;
		case MouseButton::Right:
			m_rightReleased = true;
			break;
		case MouseButton::Middle:
			m_middleReleased = true;
			break;
		}
	}
	for (auto& evt : m_queuedEvents) {
		evt();
	}
	m_previouslyUnhovered = false;
}