#ifndef EDITOR_UTIL_H
#define EDITOR_UTIL_H

#include <array>
#include <functional>
#include <locale>
#include <memory>
#include <string>
#include <thread>

#include <Windows.h>
#include <ShlObj.h>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

#include "Instance.h"

namespace nv {
	namespace editor {
		enum class EditorDest {
			None,
			Quit,
			Scene,
			Sprite,
			Text
		};
		
		template<typename Func>
		struct ScopeExit {
		private:
			Func m_f;
		public:
			ScopeExit(const Func& f) noexcept(std::is_nothrow_copy_constructible_v<Func>) : m_f{f} {}
			ScopeExit(Func&& f) noexcept(std::is_nothrow_move_constructible_v<Func>) : m_f{ std::move(f) } {}
			~ScopeExit() noexcept(std::is_nothrow_invocable_v<Func>) {
				m_f();
			}
		};

		template<typename T>
		concept RenderFuncC = std::invocable<T, Renderer&> && 
							  std::is_same_v<std::invoke_result_t<T, Renderer&>, EditorDest>;

		template<typename RenderFunc>
		EditorDest runEditor(ImGuiIO& io, Renderer& renderer, RenderFunc& showGui) 
			requires std::invocable<RenderFunc, Renderer&> && 
					 std::is_same_v<std::invoke_result_t<RenderFunc, Renderer&>, EditorDest>
		{
			while (true) {
				auto waitTime = 1000ms / NV_FPS;
				auto endTime = std::chrono::system_clock::now() + waitTime;

				SDL_Event evt;
				while (SDL_PollEvent(&evt)) {
					ImGui_ImplSDL2_ProcessEvent(&evt);
					if (evt.type == SDL_QUIT) {
						return EditorDest::Quit;
					}
				}

				ImGui_ImplSDLRenderer2_NewFrame();
				ImGui_ImplSDL2_NewFrame();
				ImGui::NewFrame();

				auto dest = showGui(renderer);

				const auto now = std::chrono::system_clock::now();

				//checks frames, render
				if (now < endTime) {
					std::this_thread::sleep_for(endTime - now);
				}

				renderer.render(io);

				if (dest != EditorDest::None) {
					return dest;
				}
			}
		}

		void runEditors();

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