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
		class EditorRenderer : public Renderer {
		private:
			plf::hive<Rect*> m_rects;
		public:
			using Renderer::Renderer;

			void render(ImGuiIO& io) noexcept;
			void addRect(Rect* rect);
			void resetBackground() noexcept;
			void moveRects(int dx, int dy) noexcept;
		};

		enum class EditorDest {
			None,
			Quit,
			Scene,
			Sprite,
			Background,
			Text,
			Home
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

		template<typename RenderMethod, typename... Events>
		EditorDest runEditor(ImGuiIO& io, EditorRenderer& renderer, RenderMethod& showGui)
			requires std::invocable<RenderMethod, EditorRenderer&> &&
					 std::is_same_v<std::invoke_result_t<RenderMethod, EditorRenderer&>, EditorDest>
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
					else if (evt.type == SDL_KEYDOWN) {
						if (evt.key.keysym.scancode == SDL_SCANCODE_MINUS) {
							renderer.resetBackground();
							renderer.clear();
							return EditorDest::Home;
						}
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

		std::optional<std::string> openFilePath();
		std::optional<std::vector<std::string>> openFilePaths();
		std::optional<std::string> saveFile(std::wstring openMessage);
		
		void loadImages(std::vector<std::string>& imagePaths, TexturePtrs& textures, Renderer& renderer);

		template<typename T>
		constexpr auto centerPos(T l1, T l2) {
			return (l1 - l2) / 2;
		}

		constexpr ImVec2 adjacentPos(const ImVec2& pos, const ImVec2& size, float spacing = 0.0f) {
			return ImVec2{ pos.x + size.x + spacing,  pos.y };
		}
		constexpr ImVec2 buttonList(const ImVec2& btnSize, int btnC) noexcept {
			return {
				btnSize.x + 15,
				btnSize.y * static_cast<float>(btnC) + 40.0f
			};
		}

		struct Coord {
			int x = 0;
			int y = 0;
		};

		//used to convert std::pair<int, int> <----> ImVec2
		template<typename Ret, typename Pair>
		Ret convertPair(const Pair& pair) noexcept {
			using Converted = std::remove_cvref_t<decltype(Ret::x)>;
			return Ret{ static_cast<Converted>(pair.x), static_cast<Converted>(pair.y) };
		}

		class RectEditor {
		private:
			using RGBA = std::array<float, 4>;
			RGBA m_floatColors{ 0.0f, 0.0f, 0.0f, 0.0f };
			bool m_dragging = false;
		public:
			Rect* rect = nullptr;
			void drag(const Coord& mousePos);
			void edit(bool showingColor = true);
		};
	}
}

#endif