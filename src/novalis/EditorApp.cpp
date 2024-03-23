#include "EditorApp.h"

void nv::editor::runEditors() {
	Instance instance{ "Novalis" };

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	auto& io = ImGui::GetIO();
	(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	ImGui::StyleColorsDark();
	ImGui_ImplSDL2_InitForSDLRenderer(instance.getRawWindow(), instance.getRawRenderer());
	ImGui_ImplSDLRenderer2_Init(instance.getRawRenderer());

	bool running = true;
	Renderer renderer{ instance.getRawRenderer() };
	EditorDest currDest = runEditor(io, renderer, runHomeEditor);
	while (running) {
		if (currDest == EditorDest::None) {
			continue;
		}
		switch (currDest) {
		case EditorDest::Sprite:
		{ 
			SpriteEditor spriteEditor{ renderer };
			currDest = runEditor(io, renderer, spriteEditor);
			break;
		}
		case EditorDest::Scene:
		{
			SceneEditor sceneEditor;
			currDest = runEditor(io, renderer, sceneEditor);
			break;
		}
		case EditorDest::Quit:
			running = false;
			break;
		}
		renderer.clear();
	}

	ImGui_ImplSDLRenderer2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}