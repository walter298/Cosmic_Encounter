#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <chrono>
#include <vector>
#include <map>
#include <functional>
#include <type_traits>
#include <string>

#include "SDL.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_sdlrenderer2.h"

#include "DataUtil.h"
#include "GlobalMacros.h"
#include "ID.h"

namespace nv {
	using EventFunc = std::move_only_function<void()>;
	using EventPred = std::move_only_function<bool()>;

	class InputHandler {
	private:
		SDL_Event m_evt;

		const Uint8* m_keyState = SDL_GetKeyboardState(nullptr);

		std::map<SDL_EventType, bool> m_inputStates;
		
		int  m_lMX = 0, m_lMY = 0;
		int  m_mouseX = 0, m_mouseY = 0;
		bool m_lastRightMouseButtonClickState = false;
		bool m_rightMouseButtonClickState = false;
		bool m_lastLeftMouseButtonClickState = false;
		bool m_leftMouseButtonClickState = false;
		bool m_lastMouseClickState = false;
		bool m_mouseClicked = false;
		bool m_mouseReleased = false;

		bool m_usingImGui = false;

		void updateMouseStates();

		InputHandler() = default;
	public:
		static InputHandler& getInstance();

		bool mouseHeld(int mask);

		template<std::same_as<SDL_EventType>... SDL_Events>
		inline bool eventStates(SDL_Events... eventTypes) requires(sizeof...(SDL_Events) >= 1) 
		{
			return (m_inputStates[eventTypes] && ...);
		}

		template<std::same_as<SDL_Scancode>... Keys>
		bool keyStates(Keys... keys) requires(sizeof...(Keys) >= 1)
		{
			return (... && m_keyState[keys]);
		}

		inline int getMouseX() {
			return m_mouseX;
		}
		inline int getMouseY() {
			return m_mouseY;
		}

		//mouse methods
		std::pair<int, int> mouseChange();
		std::pair<int, int> mouse();
		bool leftMouseClicked();
		bool leftMouseHeld();
		bool leftMouseReleased();
		bool rightMouseClicked();
		bool rightMouseHeld();
		bool rightMouseReleased();

		void run();

		void useImGui();
	};

	using namespace std::chrono;

	struct Event {
	private:
		ID<Event> m_ID;
		EventFunc m_func = [] {};

		system_clock::time_point m_time = system_clock::now();
	public:
		template<typename FuncType>
		Event(FuncType&& func, milliseconds delay = 0ms)
		{
			//if a non-zero delay value is passed in, make function have a delay counter
			if (delay != 0ms) {
				m_func = [this, func = std::move(func), delay]() mutable {
					auto now = system_clock::now();
					if (now >= m_time) {
						m_time = now + delay;
						func();
					}
				};
			} else {
				m_func = std::move(func);
			}
		}

		template<typename FuncType>
		Event(FuncType&& func, EventPred&& pred, milliseconds delay = 0ms)
			: Event(
				[func = std::move(func), pred = std::move(pred), delay]() mutable { 
					if (pred()) { func(); } 
				}, 
				delay
			)
		{}

		ID<Event> getID() const noexcept;

		void operator()();
	};
}

#endif