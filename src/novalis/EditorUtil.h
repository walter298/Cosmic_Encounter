#ifndef EDITOR_UTIL_H
#define EDITOR_UTIL_H

#include <array>
#include <functional>
#include <locale>
#include <memory>
#include <string>


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
			ImGuiInstance(SDL_Window* window, SDL_Renderer* renderer);
			~ImGuiInstance();

			ImGuiIO& getIO();
		};

		class Editor {
		private:
			ImGuiIO& m_io;
		protected:
			Renderer m_renderer;
		public:
			Editor(ImGuiIO& io, SDL_Renderer* renderer);

			void execute();
			virtual void showGui(bool&) = 0;
			virtual void customRender() {}
		};

		enum class EditorDest {
			None,
			Scene,
			Sprite,
			Text
		};

		void editRect(Rect& rect, bool editingColor = false);

		std::optional<std::string> openFilePath();
		std::optional<std::vector<std::string>> openFilePaths();
		std::optional<std::string> saveFile(std::wstring openMessage);

		template<typename T>
		constexpr auto centerPos(T l1, T l2) {
			return (l1 - l2) / 2;
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