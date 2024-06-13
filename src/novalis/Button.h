#ifndef BUTTON_H
#define BUTTON_H

#include "Event.h"
#include "ID.h"
#include "Rect.h"

namespace nv {
	class Button {
	private:
		std::vector<Event> m_queuedEvents;

		bool m_previouslyHovered = false;
		bool m_previouslyUnhovered = false;
		bool m_previouslyLeftHeld = false;
		bool m_previouslyRightHeld = false;
		bool m_previouslyMiddleHeld = false;
		bool m_leftReleased = false;
		bool m_rightReleased = false;
		bool m_middleReleased = false;

		Rect m_rect;
		ID m_ID;
	public:
		Button(const Rect& rect) noexcept;

		const ID& getID() const noexcept;

		template<std::invocable OnHovered, std::invocable OnUnhovered>
		void onHovered(OnHovered&& onHovered, OnUnhovered&& onUnhovered) {
			m_queuedEvents.emplace_back(
				[this,
				onHovered = std::forward<OnHovered>(onHovered),
				onUnhovered = std::forward<OnUnhovered>(onUnhovered)]() mutable {
					if (m_previouslyHovered) {
						onHovered();
					} else if (m_previouslyUnhovered) {
						onUnhovered();
					}
				}
			);
		}
		template<std::invocable Func>
		void onClicked(Func&& func, MouseButton btn) {
			const bool& releasedFlag = [&btn] {
				switch (btn) {
				case MouseButton::Left:
					return m_leftReleased;
					break;
				case MouseButton::Right:
					return m_rightReleased;
					break;
				default:
					return m_middleReleased;
					break;
				}
			}();
			m_queuedEvents.emplace_back(
				[&releasedFlag, func = std::forward<Func>(func)](const MouseButtonData& mouseBtnData) mutable {
					if (m_rect.containsCoord(mouseBtnData.x, mouseBtnData.y) && releasedFlag) {
						func();
					}
				}
			);
		}
		void operator()(const MouseButtonData& mouseBtnData);
	};

	using ButtonRef = std::reference_wrapper<Button>;
}

#endif