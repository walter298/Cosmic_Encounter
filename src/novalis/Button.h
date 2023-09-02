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

		Rect* m_rect = nullptr;
		ID<Button> m_ID;
	public:
		/*The render object is to be shown on the screen, and rx and ry represent the 
		screen coordinates of the render object*/
		Button(nv::Rect* rect) noexcept;

		ID<Button> getID() const noexcept;

		void onHovered(EventFunc&& func); 
		void onUnhovered(EventFunc&& func);
		void onLeftClick(EventFunc&& func);
		void onRightClick(EventFunc&& func);
		void whileLeftHeld(EventFunc&& func);

		void operator()();
	};

	using ButtonRef = std::reference_wrapper<Button>;
}

#endif