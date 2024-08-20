#ifndef BUTTON_H
#define BUTTON_H

#include "Event.h"
#include "Rect.h"
#include "Text.h"

namespace nv {
	struct ExternalRect {};
	constexpr inline ExternalRect usingExternalRect;

	class Button {
	private:
		Rect m_rect;
		std::reference_wrapper<const Rect> m_rectRef;

		Event<void> m_onClicked;
		Event<void> m_onHovered;
		Event<void> m_onUnhovered;

		bool m_previouslyHovered = false;
		bool m_previouslyUnhovered = false;
		bool m_previouslyLeftHeld = false;
		bool m_previouslyRightHeld = false;
		bool m_previouslyMiddleHeld = false;
		bool m_leftReleased = false;
		bool m_rightReleased = false;
		bool m_middleReleased = false;
	public:
		Button(const Rect& rect, Event<void>&& onClicked, Event<void>&& onHovered, Event<void>&& onUnhovered) noexcept;
		Button(ExternalRect, const Rect& rect, Event<void>&& onClicked, Event<void>&& onHovered, Event<void>&& onUnhovered) noexcept;

		void operator()(MouseData mouseBtnData);
	};
}

#endif