#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <chrono>
#include <vector>
#include <map>
#include <functional>
#include <type_traits>
#include <string>

#include <SDL2/SDL.h>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

#include "DataUtil.h"
#include "ID.h"

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

	using Keymap        = boost_con::flat_map<SDL_Scancode, bool>;
	using KeyboardEvent = Event<const Keymap&>;
}

#endif