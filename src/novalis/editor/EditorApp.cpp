#include "EditorApp.h"

#include <thread> //sleep

#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

#include "../Instance.h"
#include "HomeEditor.h"
#include "SpriteEditor.h"
#include "SceneEditor.h"

namespace {
	template<typename T>
	concept Editor = requires(T t) {
		{ t.imguiRender() } -> std::same_as<nv::editor::EditorDest>;
		t.sdlRender();
	};

	template<Editor RenderMethod>
	nv::editor::EditorDest runEditor(ImGuiIO& io, SDL_Renderer* renderer, RenderMethod& editor) {
		using namespace nv::editor;
		while (true) {
			using namespace std::literals;

			constexpr auto waitTime = 1000ms / 180;
			const auto endTime = std::chrono::system_clock::now() + waitTime;

			SDL_Event evt;
			while (SDL_PollEvent(&evt)) {
				ImGui_ImplSDL2_ProcessEvent(&evt);
				if (evt.type == SDL_QUIT) {
					return EditorDest::Quit;
				}
				else if (evt.type == SDL_KEYDOWN) {
					if (evt.key.keysym.scancode == SDL_SCANCODE_INSERT) {
						return EditorDest::Home;
					}
				}
			}

			SDL_RenderClear(renderer);
			editor.sdlRender();

			static constexpr ImVec4 color{ 0.45f, 0.55f, 0.60f, 1.00f };

			ImGui_ImplSDLRenderer2_NewFrame();
			ImGui_ImplSDL2_NewFrame();
			ImGui::NewFrame();

			auto dest = editor.imguiRender();

			const auto now = std::chrono::system_clock::now();
			if (now < endTime) {
				std::this_thread::sleep_for(endTime - now);
			}

			ImGui::Render();
			SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
			SDL_SetRenderDrawColor(renderer,
				//unfortunately SDL uses ints for screen pixels and ImGui uses floats 
				static_cast<Uint8>(color.x * 255), static_cast<Uint8>(color.y * 255),
				static_cast<Uint8>(color.z * 255), static_cast<Uint8>(color.w * 255));
			ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
			SDL_RenderPresent(renderer);
			if (dest != EditorDest::None) {
				return dest;
			}
		}
	}
}

void nv::editor::runEditors() {
	Instance instance{ "Novalis" };
	
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	auto& io = ImGui::GetIO();
	(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	ImGui::StyleColorsDark();
	ImGui_ImplSDL2_InitForSDLRenderer(instance.window, instance.renderer);
	ImGui_ImplSDLRenderer2_Init(instance.renderer);

	bool running = true;
	EditorDest currDest = EditorDest::Home;
	
	while (running) {
		if (currDest == EditorDest::None) {
			continue;
		}
		switch (currDest) {
		case EditorDest::Home:
		{
			HomeEditor homeEditor{ instance };
			currDest = runEditor(io, instance.renderer, homeEditor);
			break;
		}
		case EditorDest::Sprite:
		{
			SpriteEditor spriteEditor{ instance.renderer };
			currDest = runEditor(io, instance.renderer, spriteEditor);
			break;
		}
		case EditorDest::Scene:
		{
			SceneEditor sceneEditor{ instance.renderer };
			currDest = runEditor(io, instance.renderer, sceneEditor);
			break;
		}
		case EditorDest::Quit:
			running = false;
			break;
		}
	}
	
	ImGui_ImplSDLRenderer2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}