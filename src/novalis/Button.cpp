#include "Button.h"

nv::Button::Button(const Rect& otherRect, Event<>&& onClicked, Event<> onHovered, Event<>&& onUnhovered) noexcept
	: m_rect{ otherRect }, m_rectRef{ m_rect }, m_onClicked{ std::move(onClicked) }, 
	m_onHovered{ std::move(onHovered) }, m_onUnhovered{ std::move(onUnhovered) }
{
}

nv::Button::Button(ExternalRect, const Rect& rect, Event<>&& onClicked, Event<> onHovered, Event<>&& onUnhovered) noexcept
	: m_rectRef{ rect }, m_onClicked{ std::move(onClicked) }, 
	m_onHovered{ std::move(onHovered) }, m_onUnhovered{ std::move(onUnhovered) }
{
}

void nv::Button::operator()(MouseData mouseBtnData) {
	if (m_rectRef.get().containsCoord(mouseBtnData.x, mouseBtnData.y)) {
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