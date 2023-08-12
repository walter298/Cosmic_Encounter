#ifndef BUTTON_H
#define BUTTON_H

#include "Event.h"
#include "Rect.h"

namespace nv {
	class Button {
	private:
		std::vector<Event> m_queuedEvents;

		static int IDCount;
		bool m_previouslyHovered = false;

		Rect* m_rect = nullptr;
		int m_ID = 0;
	public:
		/*The render object is to be shown on the screen, and rx and ry represent the 
		screen coordinates of the render object*/
		Button() noexcept;
		Button(nv::Rect* rect) noexcept;

		int getID() const noexcept;

		void onHovered(EventFunc&& func); 
		void onUnhovered(EventFunc&& func);
		void onLeftClick(EventFunc&& func);
		void onRightClick(EventFunc&& func);
		void whileLeftHeld(EventFunc&& func);

		void operator()();
	};
}

#endif