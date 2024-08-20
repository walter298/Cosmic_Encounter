#pragma once

#include <functional>

#include <boost/unordered/unordered_flat_map.hpp>

#include <SDL2/SDL_scancode.h>

#include "data_util/Reflection.h"

namespace nv {
	template<typename Ret, typename... Args> 
	using Event = std::move_only_function<Ret(Args...)>;

	template<typename Func>
	concept CancellableEvent = std::same_as<bool, typename FunctionTraits<Func>::Ret>;

	class EventChain {
	private:
		std::vector<Event<bool>> m_events;
		Event<bool>* m_currEvent = nullptr;
	public:
		template<CancellableEvent... Funcs>
		EventChain(Funcs&&... funcs) requires(sizeof...(Funcs) > 0) {
			m_events.reserve(sizeof...(Funcs));
			((m_events.emplace_back(std::forward<Funcs>(funcs))), ...);
		}
		EventChain() = default;

		template<CancellableEvent Func>
		void chain(Func&& func) {
			assert(m_currEvent != nullptr); //"Error: cannot chain function after event chain has been invoked"
			m_events.emplace_back(std::forward<Func>(func));
		}

		bool operator()();
	};

	enum class MouseButtonState {
		Down,
		Released,
		None
	};
	
	struct MouseData {
		MouseButtonState left = MouseButtonState::None;
		MouseButtonState mid = MouseButtonState::None;
		MouseButtonState right = MouseButtonState::None;
		int x = 0;
		int y = 0;
		int deltaX = 0;
		int deltaY = 0;
	};

	using MouseEvent = Event<MouseData>;

	using Keymap        = boost::unordered_flat_map<SDL_Scancode, bool>;
	using KeyboardEvent = Event<const Keymap&>;
}
