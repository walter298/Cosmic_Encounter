#include "Button.h"

nv::Button::Button(const Rect& rect, Event<>&& onClicked, Event<> onHovered, Event<>&& onUnhovered) noexcept
	: m_rect{ rect }, m_onClicked{ std::move(onClicked) }, m_onHovered{ std::move(onHovered) }, m_onUnhovered{ std::move(onUnhovered) }
{
}

//void nv::Button::onClicked(Event<>&& onClicked, MouseButton btn) {
//	const bool& releasedFlag = [&, this] {
//		switch (btn) {
//		case MouseButton::Left:
//			return m_leftReleased;
//			break;
//		case MouseButton::Right:
//			return m_rightReleased;
//			break;
//		default:
//			return m_middleReleased;
//			break;
//		}
//	}();
//	m_onClicked = [&releasedFlag, onClicked = move(onClicked), this](const MouseButtonData& mouseBtnData) mutable {
//		if (m_rect.containsCoord(mouseBtnData.x, mouseBtnData.y) && releasedFlag) {
//			onClicked();
//		}
//	};
//}

void nv::Button::operator()(const MouseData& mouseBtnData) {
	if (m_rect.containsCoord(mouseBtnData.x, mouseBtnData.y)) {
		m_onHovered();
		m_previouslyHovered = true;
		if (mouseBtnData.left == MouseButtonState::Down) {
			m_onClicked();
		}
	} else if (m_previouslyHovered) {
		m_previouslyHovered = false;
		m_onUnhovered();
	}
}