#ifndef EDITOR_UTIL_H
#define EDITOR_UTIL_H

#include <array>
#include <functional>
#include <string>
#include <memory>

#include <Windows.h>
#include <ShlObj.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_sdlrenderer2.h"

#include "Scene.h"

namespace nv {
	namespace editor {
		class ImGuiInstance {
			ImGuiIO& m_io;

			ImGuiIO& initIO();
		public:
			ImGuiInstance(SDL_Window* window, Renderer& renderer);
			~ImGuiInstance();

			ImGuiIO& getIO();
		};

		void runEditor(ImGuiIO& io, Renderer& renderer, std::function<void(bool&)> showGui);

		void editRect(Rect& rect);

		std::optional<std::string> openFilePath();
		std::optional<std::string> saveFile(std::wstring openMessage);

		template<typename T>
		constexpr auto centerPos(T l1, T l2) {
			return (l1 / 2) - (l2 / 2);
		}

		using RectRef = std::reference_wrapper<nv::Rect>;

		class DragCheck {
		private:
			static bool draggingOtherObj;
			bool m_dragging = false;

			std::vector<RectRef> m_rects;
		public:
			void add(nv::Rect& rect);

			void drag();
		};
	}
}

#endif