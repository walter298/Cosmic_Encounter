#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <chrono>
#include <vector>
#include <map>
#include <functional>
#include <type_traits>
#include <string>

#include <boost/container/flat_map.hpp>

#include <SDL2/SDL.h>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

#include "DataUtil.h"
#include "GlobalMacros.h"
#include "ID.h"

namespace nv {
	namespace detail {
		template<typename... EventData> 
		class Event  {
		private:
			ID m_ID;
			std::move_only_function<void(EventData...)> m_func;
		public:
			template<std::invocable<EventData...> FuncType>
			Event(FuncType&& func) : m_func{ std::forward<FuncType>(func) }
			{
			}

			const ID& getID() const noexcept {
				return m_ID;
			}

			void operator()(const EventData&... data) noexcept {
				m_func(data...);
			}
		};
	}

	using Event = detail::Event<>;

	struct MouseMotionData {
		const int mouseX;
		const int mouseY;
		const int deltaX;
		const int deltaY;
		MouseMotionData(const SDL_MouseMotionEvent& event) noexcept;
	};
	using MouseMotionEvent = detail::Event<MouseMotionData>;
	using MouseWheelEvent = detail::Event<int>;

	enum class MouseButton {
		None,
		Left,
		Right,
		Middle
	};
	enum class MouseButtonState {
		Down,
		Released
	};
	struct MouseButtonData {
		const MouseButton btn;
		const MouseButtonState state;
		const int x;
		const int y;
		MouseButtonData(const SDL_MouseButtonEvent& evt) noexcept;
	};
	using MouseButtonEvent = detail::Event<MouseButtonData>;

	using Keymap = boost::container::flat_map<SDL_Scancode, bool>;
	using KeyboardEvent = detail::Event<const Keymap&>;

	class EventHandler {
	private:
		SDL_Event m_SDLEvt;
		Keymap m_keymap;

		std::vector<Event> m_periodicEvents;
		std::vector<MouseWheelEvent> m_mouseWheelEvents;
		std::vector<MouseButtonEvent> m_mouseButtonEvents;
		std::vector<MouseMotionEvent> m_mouseMotionEvents;
		std::vector<KeyboardEvent> m_keyboardEvents;
		std::vector<Event> m_quitEvents;

		template<typename Func>
		constexpr auto& getEventVec() noexcept {
			if constexpr (std::invocable<Func>) {
				return m_periodicEvents;
			}
			else if constexpr (std::invocable<Func, MouseWheelEvent>) {
				return m_mouseWheelEvents;
			}
			else if constexpr (std::invocable<Func, MouseButtonEvent>) {
				return m_mouseButtonEvents;
			}
			else if constexpr (std::invocable<Func, MouseMotionEvent>) {
				return m_mouseMotionEvents;
			}
			else {
				return m_keyboardEvents;
			}
		}
	public:
		EventHandler() = default;

		template<typename Func>
		void addEvent(Func&& evt) {
			auto& eventVec = getEventVec<std::remove_cvref_t<Func>>();
			eventVec.emplace_back(std::forward<Func>(evt));
		}
		template<typename Func>
		void addQuitEvent(Func&& func) {
			m_quitEvents.emplace_back(std::forward<Func>(func));
		}

		template<typename... EventData>
		void eraseEvent(const detail::Event<EventData...>& evt) {
			auto& eventVec = getEventVec<detail::Event<EventData...>>();
			auto evtPos = std::ranges::find_if(eventVec,
				[&evt](const auto& evt) {
					return evt.getID() == evt.getID();
				}
			);
			assert(evtPos != eventVec.end());
			eventVec.erase(evtPos);
		}
		void operator()() noexcept;
	};
}

#endif