#pragma once

#include <functional>

#include <boost/unordered/unordered_flat_map.hpp>

#include <SDL2/SDL_scancode.h>

namespace nv {
	template<typename... Args> 
	using Event = std::move_only_function<void(Args...)>;

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
