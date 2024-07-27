#ifndef BUTTON_H
#define BUTTON_H

#include "Event.h"
#include "ID.h"
#include "Rect.h"
#include "Text.h"

namespace nv {
	class Button {
	private:
		Event<> m_onClicked;
		Event<> m_onHovered;
		Event<> m_onUnhovered;

		bool m_previouslyHovered = false;
		bool m_previouslyUnhovered = false;
		bool m_previouslyLeftHeld = false;
		bool m_previouslyRightHeld = false;
		bool m_previouslyMiddleHeld = false;
		bool m_leftReleased = false;
		bool m_rightReleased = false;
		bool m_middleReleased = false;

		Rect m_rect;
	public:
		Button(const Rect& rect, Event<>&& onClicked, Event<> onHovered, Event<>&& onUnhovered) noexcept;

		void operator()(const MouseData& mouseBtnData);
	};
}

#endif